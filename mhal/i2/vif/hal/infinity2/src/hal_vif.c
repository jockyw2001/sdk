#include <linux/types.h>
#include <ms_platform.h>
#include <vif_datatype.h>
#include "hal_vif.h"
#include "arch/infinity2_reg_padtop1.h"
#include "arch/infinity2_reg_block_ispsc.h"
#include "arch/infinity2_reg_clkgen2.h"
#include <vif_common.h>
#include <hal_dma_private.h>

unsigned int *g_VIFReg[VIF_CHANNEL_NUM] = {	NULL,NULL,NULL,NULL,
										        NULL,NULL,NULL,NULL,
											NULL,NULL,NULL,NULL,
											NULL,NULL,NULL,NULL};

extern void *g_ISP_ClkGen;

void HalVif_EnableIrq(VIF_CHANNEL_e ch)
{
	VdmaHandle_t *pstHnd = (VdmaHandle_t*) &gstHandle;

	pstHnd->irq->reg_c_irq_mask2 &= ~(0x1<<ch);

	return;
}

u32 HalVifGetGroupByChn(VIF_CHANNEL_e ch)
{
    return (ch >> 2);
}

u32 HalVifGetChnByGroup(u32 group)
{
    return (group << 2);
}

u32 HalVifGetHDRChnPair(VIF_CHANNEL_e ch)
{
    switch (ch) {
        case 0:
            return 4;
        case 4:
            return 0;
        default:
            VIF_DEBUG("[%s] Error, No HDR pair channel \n", __func__);
            return 0;
    }
}

VIF_STATUS_e HalVif_SetVifChanelBaseAddr(VIF_CHANNEL_e ch)
{
	VIF_STATUS_e ret = VIF_SUCCESS;
	switch(ch)
	{
		case VIF_CHANNEL_0:
		case VIF_CHANNEL_1:
			g_VIFReg[ch] = g_ptVIF;
			break;
		case VIF_CHANNEL_2:
		case VIF_CHANNEL_3:
			g_VIFReg[ch] = g_ptVIF + 0x200;
			break;
		case VIF_CHANNEL_4:
		case VIF_CHANNEL_5:
			g_VIFReg[ch] = g_ptVIF + 0x400;
			break;
		case VIF_CHANNEL_6:
		case VIF_CHANNEL_7:
			g_VIFReg[ch] = g_ptVIF + 0x600;
			break;
		case VIF_CHANNEL_8:
		case VIF_CHANNEL_9:
			g_VIFReg[ch] = g_ptVIF + 0x800;
			break;
		case VIF_CHANNEL_10:
		case VIF_CHANNEL_11:
			g_VIFReg[ch] = g_ptVIF + 0xA00;
			break;
		case VIF_CHANNEL_12:
		case VIF_CHANNEL_13:
			g_VIFReg[ch] = g_ptVIF2;
			break;
		case VIF_CHANNEL_14:
		case VIF_CHANNEL_15:
			g_VIFReg[ch] = g_ptVIF2 + 0x200;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
			ret = VIF_FAIL;
	}
	pr_info("[%s] g_VIFReg[%d]: %p\n", __func__, ch, g_VIFReg[ch]);

    return ret;
}


void HalVif_SensorSWReset(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
		    pvif_handle->reg_vif_ch0_sensor_sw_rstz = OnOff;
			break;
		case VIF_CHANNEL_1:
		    pvif_handle->reg_vif_ch1_sensor_sw_rstz = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_IFStatusReset(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
		    pvif_handle->reg_vif_ch0_if_state_rst = OnOff;
			break;
		case VIF_CHANNEL_1:
		    pvif_handle->reg_vif_ch1_if_state_rst = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SensorReset(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_rst = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_rst = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SensorPowerDown(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_pwrdn = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_pwrdn = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

s32 HalVifDataIeConfig(u32 group, u32 nValue)
{
    volatile infinity2_reg_padtop1* padtop = (infinity2_reg_padtop1*)g_TOPPAD1;
    switch(group)
    {
        case 0:
            padtop->reg_snr0_d_ie = nValue;
            break;
        case 1:
            padtop->reg_snr1_d_ie = nValue;
            break;
        case 2:
            padtop->reg_snr2_d_ie = nValue;
            break;
        case 3:
            padtop->reg_snr3_d_ie = nValue;
            break;
        default:
            VIF_DEBUG("[VIF] Invalid channel for mipi sensor.");
            return E_HAL_VIF_ERROR;
    }
    return E_HAL_VIF_SUCCESS;
}

void HalVif_Ccir8BitDataPadConfig(VIF_CHANNEL_e ch,u32 nValue){

    volatile infinity2_reg_padtop1* padtop = (infinity2_reg_padtop1*) g_TOPPAD1;

    if(ch == 0){
        padtop->reg_ccir0_8b_mode = nValue;

    }else if(ch == 1){
        padtop->reg_ccir1_8b_mode = nValue;

    }else if(ch == 2){
        padtop->reg_ccir2_8b_mode = nValue;
    }else if(ch == 3){
        padtop->reg_ccir3_8b_mode = nValue;

    }else{


    }

}

void HalVif_Ccir16BitDataPadConfig(VIF_CHANNEL_e ch,u32 nValue){

    volatile infinity2_reg_padtop1* padtop = (infinity2_reg_padtop1*) g_TOPPAD1;

    if(ch == 0){
        padtop->reg_ccir0_16b_mode = nValue;

    }else if(ch == 2){
        padtop->reg_ccir2_16b_mode = nValue;
    }else{


    }

}

void HalVif_CcirCtrlPadConfig(VIF_CHANNEL_e ch,u32 nValue){

    volatile infinity2_reg_padtop1* padtop = (infinity2_reg_padtop1*) g_TOPPAD1;

    if(ch == 0){
        padtop->reg_ccir0_ctrl_mode = nValue;

    }else if(ch == 1){
        padtop->reg_ccir1_ctrl_mode = nValue;

    }else if(ch == 2){
        padtop->reg_ccir2_ctrl_mode = nValue;
    }else if(ch == 3){

        padtop->reg_ccir3_ctrl_mode = nValue;
    }else{

    }

    return;
}

void HalVif_CcirPclkPadConfig(VIF_CHANNEL_e ch,u32 nValue){

    volatile infinity2_reg_padtop1* padtop = (infinity2_reg_padtop1*) g_TOPPAD1;

    if(ch == 0){
        padtop->reg_ccir0_clk_mode = nValue;
    }else if(ch == 1){
        padtop->reg_ccir1_clk_mode = nValue;
    }else if(ch == 2){
        padtop->reg_ccir2_clk_mode = nValue;
    }else if(ch == 3){
        padtop->reg_ccir3_clk_mode = nValue;

    }else{


    }

    return;
}


void HalVif_ChannelAlignment(VIF_CHANNEL_e ch){

    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
    volatile infinity2_reg_block_ispsc* vifclk = (infinity2_reg_block_ispsc*) g_ISP_ClkGen;
    //int cnt =0;
    u16 tmp[4] = {0};
    VIF_MS_SLEEP(100);

    printk("============= ch:%d reg_vif_ch0_bt656_ch=%d==============\n",ch,pvif_handle->reg_vif_ch0_bt656_ch);

    switch(ch)
    {
    case VIF_CHANNEL_0:

        tmp[0] = vifclk->reg_ckg_snr0;
        tmp[1] = vifclk->reg_ckg_snr1;
        tmp[2] = vifclk->reg_ckg_snr2;
        tmp[3] = vifclk->reg_ckg_snr3;

        if(pvif_handle->reg_vif_ch0_bt656_ch == 1){
            vifclk->reg_ckg_snr0 =  tmp[3];
            vifclk->reg_ckg_snr1 =  tmp[0];
            vifclk->reg_ckg_snr2 =  tmp[1];
            vifclk->reg_ckg_snr3 =  tmp[2];
        }
        else if(pvif_handle->reg_vif_ch0_bt656_ch == 2){
            vifclk->reg_ckg_snr0 = tmp[2];
            vifclk->reg_ckg_snr1 = tmp[3];
            vifclk->reg_ckg_snr2 = tmp[0];
            vifclk->reg_ckg_snr2 = tmp[1];
        }
        else if(pvif_handle->reg_vif_ch0_bt656_ch == 3){
            vifclk->reg_ckg_snr0 = tmp[1];
            vifclk->reg_ckg_snr1 = tmp[2];
            vifclk->reg_ckg_snr2 = tmp[3];
            vifclk->reg_ckg_snr2 = tmp[0];
        }
        else{

            //do nothing
        }
    break;
    case VIF_CHANNEL_4:

        tmp[0] = vifclk->reg_ckg_snr4;
        tmp[1] = vifclk->reg_ckg_snr5;
        tmp[2] = vifclk->reg_ckg_snr6;
        tmp[3] = vifclk->reg_ckg_snr7;

        if(pvif_handle->reg_vif_ch0_bt656_ch == 1){
            vifclk->reg_ckg_snr4 =  tmp[3];
            vifclk->reg_ckg_snr5 =  tmp[0];
            vifclk->reg_ckg_snr6 =  tmp[1];
            vifclk->reg_ckg_snr7 =  tmp[2];
        }
        else if(pvif_handle->reg_vif_ch0_bt656_ch == 2){
            vifclk->reg_ckg_snr4 = tmp[2];
            vifclk->reg_ckg_snr5 = tmp[3];
            vifclk->reg_ckg_snr6 = tmp[0];
            vifclk->reg_ckg_snr7 = tmp[1];
        }
        else if(pvif_handle->reg_vif_ch0_bt656_ch == 3){
            vifclk->reg_ckg_snr4 = tmp[1];
            vifclk->reg_ckg_snr5 = tmp[2];
            vifclk->reg_ckg_snr6 = tmp[3];
            vifclk->reg_ckg_snr7 = tmp[0];
        }
        else{

            //do nothing
        }
    break;
    case VIF_CHANNEL_8:

        tmp[0] = vifclk->reg_ckg_snr8;
        tmp[1] = vifclk->reg_ckg_snr9;
        tmp[2] = vifclk->reg_ckg_snr10;
        tmp[3] = vifclk->reg_ckg_snr11;

        if(pvif_handle->reg_vif_ch0_bt656_ch == 1){
            vifclk->reg_ckg_snr8 =  tmp[3];
            vifclk->reg_ckg_snr9 =  tmp[0];
            vifclk->reg_ckg_snr10 =  tmp[1];
            vifclk->reg_ckg_snr11 =  tmp[2];
        }
        else if(pvif_handle->reg_vif_ch0_bt656_ch == 2){
            vifclk->reg_ckg_snr8 = tmp[2];
            vifclk->reg_ckg_snr9 = tmp[3];
            vifclk->reg_ckg_snr10 = tmp[0];
            vifclk->reg_ckg_snr11 = tmp[1];
        }
        else if(pvif_handle->reg_vif_ch0_bt656_ch == 3){
            vifclk->reg_ckg_snr8 = tmp[1];
            vifclk->reg_ckg_snr9 = tmp[2];
            vifclk->reg_ckg_snr10 = tmp[3];
            vifclk->reg_ckg_snr11 = tmp[0];
        }
        else{

            //do nothing
        }
    break;
    case VIF_CHANNEL_12:

        tmp[0] = vifclk->reg_ckg_snr12;
        tmp[1] = vifclk->reg_ckg_snr13;
        tmp[2] = vifclk->reg_ckg_snr14;
        tmp[3] = vifclk->reg_ckg_snr15;

        if(pvif_handle->reg_vif_ch0_bt656_ch == 1){
            vifclk->reg_ckg_snr12 =  tmp[3];
            vifclk->reg_ckg_snr13 =  tmp[0];
            vifclk->reg_ckg_snr14 =  tmp[1];
            vifclk->reg_ckg_snr15 =  tmp[2];
        }
        else if(pvif_handle->reg_vif_ch0_bt656_ch == 2){
            vifclk->reg_ckg_snr12 = tmp[2];
            vifclk->reg_ckg_snr13 = tmp[3];
            vifclk->reg_ckg_snr14 = tmp[0];
            vifclk->reg_ckg_snr15 = tmp[1];
        }
        else if(pvif_handle->reg_vif_ch0_bt656_ch == 3){
            vifclk->reg_ckg_snr12 = tmp[1];
            vifclk->reg_ckg_snr13 = tmp[2];
            vifclk->reg_ckg_snr14 = tmp[3];
            vifclk->reg_ckg_snr15 = tmp[0];
        }
        else{

            //do nothing
        }
    break;
    default:
        pr_info("[%s] err, out off channel\n", __func__);
    break;
    }


}

void HalVif_SensorChannelEnable(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;


    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_en = OnOff;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_en = OnOff;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

void HalVif_HDRen(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_hdr_en = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_hdr_en = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_HDRSelect(VIF_CHANNEL_e ch, VIF_HDR_SOURCE_e src)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_hdr_sel = src;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_hdr_sel = src;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

VIF_ONOFF_e HalVif_SensorMask(VIF_CHANNEL_e ch,VIF_ONOFF_e OnOff)
{
    int i=0;
    VIF_ONOFF_e eCurVal;
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
		    eCurVal = pvif_handle->reg_vif_ch0_sensor_mask;
			pvif_handle->reg_vif_ch0_sensor_mask = OnOff;
			break;
		case VIF_CHANNEL_1:
		    eCurVal = pvif_handle->reg_vif_ch1_sensor_mask;
			pvif_handle->reg_vif_ch1_sensor_mask = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}

	for(i=0; i< 17 ; i++){

	    if(pvif_handle->reg_vif_ch0_sensor_mask == OnOff)  //check mask success, polling every 170ms for 6fps one frame period
	        return eCurVal;

	    VIF_MS_SLEEP(10);

	}

	pr_info("[%s] wait sensor mask timeout\n", __func__);

	return eCurVal;
}

void HalVif_IfDeMode(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_if_de_mode = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_if_de_mode = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}

}

void HalVif_EnSWStrobe(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_en_sw_strobe = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_en_sw_strobe = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}

}

void HalVif_SWStrobe(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sw_strobe = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sw_strobe = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}

}

void HalVif_StrobePolarity(VIF_CHANNEL_e ch, VIF_STROBE_POLARITY_e polarity)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_strobe_polarity = polarity;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_strobe_polarity = polarity;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_StrobeStart(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_strobe_start = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_strobe_start = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_StrobeRef(VIF_CHANNEL_e ch, VIF_STROBE_VERTICAL_START_e startType)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_strobe_ref = startType;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_strobe_ref = startType;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

void HalVif_StrobeEnd(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_strobe_end = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_strobe_end = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_EnHWStrobe(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_en_hw_strobe = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_en_hw_strobe = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_HWStrobeCNT(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_hw_strobe_cnt = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_hw_strobe_cnt = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_HWLongStrobeEndRef(VIF_CHANNEL_e ch, VIF_STROBE_VERTICAL_START_e startType)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_hw_long_strobe_end_ref = startType;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_hw_long_strobe_end_ref = startType;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_HWStrobeMode(VIF_CHANNEL_e ch, VIF_STROBE_MODE_e strobMode)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_hw_strobe_mode = strobMode;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_hw_strobe_mode = strobMode;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SensorFormatLeftSht(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_format_left_sht = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_format_left_sht = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SensorBitSwap(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_bit_swap = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_bit_swap = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SensorHsyncPolarity(VIF_CHANNEL_e ch, VIF_SENSOR_POLARITY_e polarity)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_hsync_polarity = polarity;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_hsync_polarity = polarity;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SensorVsyncPolarity(VIF_CHANNEL_e ch, VIF_SENSOR_POLARITY_e polarity)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_vsync_polarity = polarity;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_vsync_polarity = polarity;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SensorPclkPolarity(VIF_CHANNEL_e ch, VIF_SENSOR_POLARITY_e polarity)
{
#if 0
    ScClkCtl_t *psc_clk_ctl_handle = g_ptScClkCtl;
    unsigned int val = (polarity==VIF_SENSOR_POLARITY_HIGH_ACTIVE)?0x0:0x02;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            psc_clk_ctl_handle->reg_ckg_snr0 = (psc_clk_ctl_handle->reg_ckg_snr0&0xFC) | val;
            break;
        case VIF_CHANNEL_1:
            psc_clk_ctl_handle->reg_ckg_snr1 = (psc_clk_ctl_handle->reg_ckg_snr1&0xFC) | val;
            break;
        default:
            pr_info("[%s] err, over VIF channel number \n", __func__);
    }
#endif
}

void HalVif_SensorFormat(VIF_CHANNEL_e ch, VIF_SENSOR_FORMAT_e format)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_format = format;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_format = format;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SelectSource(VIF_CHANNEL_e ch,VIF_CHANNEL_SOURCE_e src)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;
#if 0
    infinity2_reg_ispsc_vif *pvif_handle = g_ptVIF; //RTK
    //ScClkCtl_t *psc_clk_ctl_handle = g_ptScClkCtl;

    unsigned int pclk = 0;

    psc_clk_ctl_handle->reg_ckg_bt656_0 = 0x0C; //TBD
    psc_clk_ctl_handle->reg_ckg_bt656_1 = 0x0C; //TBD

    switch(src)
    {
        case VIF_CH_SRC_PARALLEL_SENSOR_0:
        case VIF_CH_SRC_PARALLEL_SENSOR_1:
            pclk = VIF_CLK_BT656_P0_0_P;//VIF_CLK_BT656_P1_0_P; //VIF_CLK_PARALLEL; //TBD. Ask designer
            break;
        case VIF_CH_SRC_MIPI_0:
        case VIF_CH_SRC_MIPI0_VC0:
        case VIF_CH_SRC_MIPI0_VC1:
        case VIF_CH_SRC_MIPI0_VC2:
        case VIF_CH_SRC_MIPI0_VC3:
            pclk = VIF_CLK_CSI2_MAC_0;
            break;
        case VIF_CH_SRC_MIPI_1:
        case VIF_CH_SRC_MIPI1_VC0:
        case VIF_CH_SRC_MIPI1_VC1:
        case VIF_CH_SRC_MIPI1_VC2:
        case VIF_CH_SRC_MIPI1_VC3:
            pclk = VIF_CLK_CSI2_MAC_1;
            break;
        case VIF_CH_SRC_BT656:
            //psc_clk_ctl_handle->reg_ckg_bt656_1 = 0x00; //Dual channel //TBD

#if 0//defined(__MV5_FPGA__)
            pclk = VIF_CLK_FPGA_BT656; //TBD
#else
            if(ch == VIF_CHANNEL_1){
                pclk = VIF_CLK_BT656_P1_0_P;//VIF_CLK_BT656_P1_1_P;//
            }
#endif
            break;
        case VIF_CH_SRC_BT601:
            pclk = VIF_CLK_PARALLEL; //TBD
            break;
    }
#endif
    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_src_sel = src;
            //psc_clk_ctl_handle->reg_ckg_snr0 = (psc_clk_ctl_handle->reg_ckg_snr0&0x3) | (pclk<<2);
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_src_sel = src;
            //psc_clk_ctl_handle->reg_ckg_snr1 = (psc_clk_ctl_handle->reg_ckg_snr1&0x3) | (pclk<<2);
            break;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

void HalVif_SensorRgbIn(VIF_CHANNEL_e ch, VIF_SENSOR_INPUT_FORMAT_e format)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_rgb_in = format;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_rgb_in = format;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SensorFormatExtMode(VIF_CHANNEL_e ch, VIF_SENSOR_BIT_MODE_e mode)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_format_ext_mode = mode;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_format_ext_mode = mode;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SensorYc16Bit(VIF_CHANNEL_e ch, VIF_SENSOR_YC_INPUT_FORMAT_e format)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_yc16bit = format;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_yc16bit = format;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SensorVsDly(VIF_CHANNEL_e ch, VIF_SENSOR_VS_DELAY_e delay)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_vs_dly = delay;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_vs_dly = delay;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_SensorHsDly(VIF_CHANNEL_e ch, VIF_SENSOR_HS_DELAY_e delay)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_sensor_hs_dly = delay;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_sensor_hs_dly = delay;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}
#if 0
void HalVif_ParWidth(VIF_CHANNEL_e ch, unsigned int width)
{
    infinity2_reg_ispsc_vif *pvif_handle = g_ptVIF; //RTK

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_par_width = width;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_par_width = width;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_Bt601ByteintEn(VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = g_ptVIF; //RTK

	pvif_handle->reg_bt601_byteint_en = OnOff;
}

void HalVif_Bt601ChSel(VIF_CHANNEL_e ch, unsigned int channel)
{
    infinity2_reg_ispsc_vif *pvif_handle = g_ptVIF; //RTK

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_bt601_ch_sel = channel;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_bt601_ch_sel= channel;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}
#endif

void HalVif_Bt656Yc16bitMode(VIF_CHANNEL_e ch, VIF_SENSOR_BT656_FORMAT_e format)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_bt656_yc16bit_mode = format;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_bt656_yc16bit_mode = format;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_Bt1120Yc16bitMode(VIF_CHANNEL_e ch, VIF_SENSOR_BT656_FORMAT_e format)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_bt1120_yc16bit_mode = format;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_bt1120_yc16bit_mode = format;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_Bt656Chanel(VIF_CHANNEL_e ch, unsigned int bt656_ch)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_bt656_ch = bt656_ch;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_bt656_ch = bt656_ch;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_Bt656Field(VIF_CHANNEL_e ch, unsigned int bt656_fd)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_bt656_fd = bt656_fd;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_bt656_fd = bt656_fd;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}


void HalVif_DebugSel(unsigned int sel)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[VIF_CHANNEL_0];

	pvif_handle->reg_vif_debug_sel = sel;
}

void HalVif_CaptureEn(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_capture_en = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_capture_en = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_IrqMask(VIF_CHANNEL_e ch, unsigned int mask)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_c_irq_mask = mask;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_c_irq_mask = mask;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_IrqUnMask(VIF_CHANNEL_e ch, unsigned int mask)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_c_irq_mask &= (~mask);
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_c_irq_mask &= (~mask);
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}


void HalVif_IrqForce(VIF_CHANNEL_e ch, unsigned int mask)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_c_irq_force = mask;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_c_irq_force = mask;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_IrqClr(VIF_CHANNEL_e ch, unsigned int mask)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_c_irq_clr &= mask;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_c_irq_clr &= mask;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_IrqClr1(VIF_CHANNEL_e ch, unsigned int mask)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_c_irq_clr |= mask;
            pvif_handle->reg_vif_ch0_c_irq_clr &= ~mask;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_c_irq_clr |= mask;
            pvif_handle->reg_vif_ch1_c_irq_clr &= ~mask;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}


unsigned int HalVif_IrqFinalStatus(VIF_CHANNEL_e ch)
{
	unsigned int u32IntFinalStatus = 0;
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			u32IntFinalStatus = pvif_handle->reg_vif_ch0_irq_final_status;
			break;
		case VIF_CHANNEL_1:
			u32IntFinalStatus = pvif_handle->reg_vif_ch1_irq_final_status;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}

	return u32IntFinalStatus;
}

unsigned int HalVif_IrqRawStatus(VIF_CHANNEL_e ch)
{
	unsigned int u32IntRawStatus = 0;
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			u32IntRawStatus = pvif_handle->reg_vif_ch0_irq_raw_status;
			break;
		case VIF_CHANNEL_1:
			u32IntRawStatus = pvif_handle->reg_vif_ch1_irq_raw_status;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}

	return u32IntRawStatus;
}

void HalVif_Crop(VIF_CHANNEL_e ch, unsigned int x,unsigned int y,unsigned int width,unsigned int height)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
    ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pix_crop_st = x;
            pvif_handle->reg_vif_ch0_pix_crop_end = x + width-1;
            pvif_handle->reg_vif_ch0_line_crop_st = y;
            pvif_handle->reg_vif_ch0_line_crop_end = y + height-1;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pix_crop_st = x;
            pvif_handle->reg_vif_ch1_pix_crop_end = x + width-1;
            pvif_handle->reg_vif_ch1_line_crop_st = y;
            pvif_handle->reg_vif_ch1_line_crop_end = y + height-1;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }


}


void HalVif_PixCropStart(VIF_CHANNEL_e ch, unsigned int crop_start)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_pix_crop_st = crop_start;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_pix_crop_st = crop_start;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PixCropEnd(VIF_CHANNEL_e ch, unsigned int crop_end)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_pix_crop_end = crop_end;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_pix_crop_end = crop_end;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_CropEnable(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_crop_en = OnOff;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_crop_en = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_LineCropStart(VIF_CHANNEL_e ch, unsigned int crop_start)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_line_crop_st = crop_start;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_line_crop_st = crop_start;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_LineCropEnd(VIF_CHANNEL_e ch, unsigned int crop_end)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			pvif_handle->reg_vif_ch0_line_crop_end = crop_end;
			break;
		case VIF_CHANNEL_1:
			pvif_handle->reg_vif_ch1_line_crop_end = crop_end;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}
void HalVif_Vif2IspLineCnt0(VIF_CHANNEL_e ch, unsigned int LineCnt)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_vif2isp_line_cnt0 = LineCnt;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_vif2isp_line_cnt0 = LineCnt;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_Vif2IspLineCnt1(VIF_CHANNEL_e ch, unsigned int LineCnt)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

	switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_vif2isp_line_cnt1 = LineCnt;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_vif2isp_line_cnt1 = LineCnt;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_LiSt(VIF_CHANNEL_e ch, unsigned int line)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_li_st = line;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_li_st = line;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

#if 0
void HalVif_LiSkip4data(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = g_ptVIF; //RTK

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_li_skip_4data = OnOff;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_li_skip_4data = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}
#endif

void HalVif_LiCh(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_li_ch = OnOff;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_li_ch = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_LiModeEn(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_li_mode_en = OnOff;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_li_mode_en = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_LiEnd(VIF_CHANNEL_e ch, unsigned int line)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_li_end = line;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_li_end = line;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_TotalPixelCount(VIF_CHANNEL_e ch, unsigned int pixel_cnt)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_total_pix_cnt = pixel_cnt;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_total_pix_cnt = pixel_cnt;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_TotalLineCount(VIF_CHANNEL_e ch, unsigned int line_cnt)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_total_line_cnt = line_cnt;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_total_line_cnt = line_cnt;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatTgenEn(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_tgen_en = OnOff;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_tgen_en = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatDgenEn(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_dgen_en = OnOff;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_dgen_en = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatHsgenEn(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_hsgen_en = OnOff;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_hsgen_en = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatDgenRst(VIF_CHANNEL_e ch, VIF_ONOFF_e OnOff)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_dgen_rst = OnOff;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_dgen_rst = OnOff;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatSensorArray(VIF_CHANNEL_e ch, VIF_BAYER_MODE_e bayer)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_sensor_array = bayer;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_sensor_array = bayer;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatDeRate(VIF_CHANNEL_e ch, int rate)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_de_rate = rate;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_de_rate = rate;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatImgWidthM1(VIF_CHANNEL_e ch, unsigned int width)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_img_width_m1 = width;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_img_width_m1 = width;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatImgHeightM1(VIF_CHANNEL_e ch, unsigned int height)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_img_height_m1 = height;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_img_height_m1 = height;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatVsLine(VIF_CHANNEL_e ch, unsigned int line)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_vs_line = line;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_vs_line = line;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatHsPxl(VIF_CHANNEL_e ch, unsigned int pixel)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_hs_pxl = pixel;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_hs_pxl = pixel;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatHblank(VIF_CHANNEL_e ch, unsigned int blankTime)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_hblank = blankTime;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_hblank = blankTime;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatVblank(VIF_CHANNEL_e ch, unsigned int blankTime)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_vblank = blankTime;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_vblank = blankTime;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatBlkWidthM1(VIF_CHANNEL_e ch, unsigned int width)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_blk_width_m1 = width;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_blk_width_m1 = width;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatBlkHeightM1(VIF_CHANNEL_e ch, unsigned int height)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_blk_height_m1 = height;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_blk_height_m1 = height;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatColorInitIdx(VIF_CHANNEL_e ch, unsigned int colorBarIndex)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_color_init_idx = colorBarIndex;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_color_init_idx = colorBarIndex;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatColorPercent(VIF_CHANNEL_e ch, unsigned int colorPercent)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_color_percent = colorPercent;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_color_percent = colorPercent;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatFrmChgRate(VIF_CHANNEL_e ch, unsigned int changeRate)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_frm_chg_rate = changeRate;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_frm_chg_rate = changeRate;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatFrmChgX(VIF_CHANNEL_e ch, unsigned int shiftPixel)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_frm_chg_x = shiftPixel;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_frm_chg_x = shiftPixel;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_PatFrmChgY(VIF_CHANNEL_e ch, unsigned int shiftPixel)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_pat_frm_chg_y = shiftPixel;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_pat_frm_chg_y = shiftPixel;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_InputHsCnt(VIF_CHANNEL_e ch, unsigned int HSYNCCnt)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_input_hs_cnt = HSYNCCnt;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_input_hs_cnt = HSYNCCnt;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_InputDeCnt(VIF_CHANNEL_e ch, unsigned int DECnt)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
	{
		case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_input_de_cnt = DECnt;
			break;
		case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_input_de_cnt = DECnt;
			break;
		default:
			pr_info("[%s] err, over VIF chanel number \n", __func__);
	}
}

void HalVif_BT656InputSelect(VIF_CHANNEL_e ch, unsigned int bt656_input_sel)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_input_sel = bt656_input_sel;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_input_sel = bt656_input_sel;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

void HalVif_BT656ChannelDetectEnable(VIF_CHANNEL_e ch, VIF_ONOFF_e bt656ch_det_en)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_ch_det_en = bt656ch_det_en;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_ch_det_en = bt656ch_det_en;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

void HalVif_BT656ChannelDetectSelect(VIF_CHANNEL_e ch, VIF_BT656_CHANNEL_SELECT_e bt656ch_det_sel)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_ch_det_sel = bt656ch_det_sel;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_ch_det_sel = bt656ch_det_sel;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

void HalVif_BT656BitSwap(VIF_CHANNEL_e ch, unsigned int bt656_bit_swap)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_bit_swap = bt656_bit_swap;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_bit_swap = bt656_bit_swap;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

#if 0
void HalVif_BT6568BitMode(VIF_CHANNEL_e ch, unsigned int bt656_8bit_mode)
{
    infinity2_reg_ispsc_vif *pvif_handle = g_ptVIF; //RTK

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_8bit_mode = bt656_8bit_mode;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_8bit_mode = bt656_8bit_mode;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}
#endif

#if 0
void HalVif_BT6568BitExt(VIF_CHANNEL_e ch, unsigned int bt656_8bit_ext)
{
    infinity2_reg_ispsc_vif *pvif_handle = g_ptVIF; //RTK

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_8bit_ext = bt656_8bit_ext;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_8bit_ext = bt656_8bit_ext;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}
#endif

void HalVif_BT656VSDelay(VIF_CHANNEL_e ch, VIF_BT656_VSYNC_DELAY_e bt656_vs_delay)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_vs_sel = bt656_vs_delay;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_vs_sel = bt656_vs_delay;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

void HalVif_BT656HsyncInvert(VIF_CHANNEL_e ch, unsigned int bt656_hsync_inv)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_hs_inv = bt656_hsync_inv;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_hs_inv = bt656_hsync_inv;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

void HalVif_BT656VsyncInvert(VIF_CHANNEL_e ch, unsigned int bt656_vsync_inv)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_vs_inv = bt656_vsync_inv;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_vs_inv = bt656_vsync_inv;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

void HalVif_BT656ClampEnable(VIF_CHANNEL_e ch, unsigned int bt656_clamp_en)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_clamp_en = bt656_clamp_en;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_clamp_en = bt656_clamp_en;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}


unsigned int HalVif_Bt656DetStatus0(VIF_CHANNEL_e ch)
{
	unsigned int detStatus = 0;
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            detStatus = pvif_handle->reg_vif_ch0_bt656_det_status0;
            break;
        case VIF_CHANNEL_1:
            detStatus = pvif_handle->reg_vif_ch1_bt656_det_status0;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
	return detStatus;
}

unsigned int HalVif_Bt656DetStatus1(VIF_CHANNEL_e ch)
{
	unsigned int detStatus = 0;
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            detStatus = pvif_handle->reg_vif_ch0_bt656_det_status1;
            break;
        case VIF_CHANNEL_1:
            detStatus = pvif_handle->reg_vif_ch1_bt656_det_status1;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
	return detStatus;
}

void HalVif_BT656VerticalCropSize(VIF_CHANNEL_e ch, unsigned int bt656_vertical_crop_size)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_crop_v = bt656_vertical_crop_size;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_crop_v = bt656_vertical_crop_size;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

void HalVif_BT656HorizontalCropSize(VIF_CHANNEL_e ch, unsigned int bt656_horizontal_crop_size)
{
    infinity2_reg_ispsc_vif *pvif_handle = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
	ch %= 2;

    switch(ch)
    {
        case VIF_CHANNEL_0:
            pvif_handle->reg_vif_ch0_bt656_crop_h = bt656_horizontal_crop_size;
            break;
        case VIF_CHANNEL_1:
            pvif_handle->reg_vif_ch1_bt656_crop_h = bt656_horizontal_crop_size;
            break;
        default:
            pr_info("[%s] err, over VIF chanel number \n", __func__);
    }
}

void HalVif_SetPclkSource(VIF_CHANNEL_e ch,VifPclk_e eClk, u8 inverse, u8 gating)
{
    volatile infinity2_reg_block_ispsc* vifclk = (infinity2_reg_block_ispsc*) g_ISP_ClkGen;
    u32 clk = (eClk<<2) | ((inverse&0x1)<<1) | (gating&0x1);
    switch(ch)
    {
    case VIF_CHANNEL_0:
        vifclk->reg_ckg_snr0 = clk;
        break;
    case VIF_CHANNEL_1:
        vifclk->reg_ckg_snr1 = clk;
        break;
    case VIF_CHANNEL_2:
        vifclk->reg_ckg_snr2 = clk;
        break;
    case VIF_CHANNEL_3:
        vifclk->reg_ckg_snr3 = clk;
        break;
    case VIF_CHANNEL_4:
        vifclk->reg_ckg_snr4 = clk;
        break;
    case VIF_CHANNEL_5:
        vifclk->reg_ckg_snr5 = clk;
        break;
    case VIF_CHANNEL_6:
        vifclk->reg_ckg_snr6 = clk;
        break;
    case VIF_CHANNEL_7:
        vifclk->reg_ckg_snr7 = clk;
        break;
    case VIF_CHANNEL_8:
        vifclk->reg_ckg_snr8 = clk;
        break;
    case VIF_CHANNEL_9:
        vifclk->reg_ckg_snr9 = clk;
        break;
    case VIF_CHANNEL_10:
        vifclk->reg_ckg_snr10 = clk;
        break;
    case VIF_CHANNEL_11:
        vifclk->reg_ckg_snr11 = clk;
        break;
    case VIF_CHANNEL_12:
        vifclk->reg_ckg_snr12 = clk;
        break;
    case VIF_CHANNEL_13:
        vifclk->reg_ckg_snr13 = clk;
        break;
    case VIF_CHANNEL_14:
        vifclk->reg_ckg_snr14 = clk;
        break;
    case VIF_CHANNEL_15:
        vifclk->reg_ckg_snr15 = clk;
        break;
    default:
        pr_err("[VIF] Invalid VIF channel for PCLK.");
        break;
    }
}

void HalVif_SetMCLK(VIF_CHANNEL_e ch,VifMclk_e eClk)
{
    volatile infinity2_reg_clkgen2* clkgen = (infinity2_reg_clkgen2*) g_CLKGEN2;
    switch(ch)
    {
    case 0:
        clkgen->reg_ckg_sr0_mclk = (eClk<<2);
        break;
    case 4:
        clkgen->reg_ckg_sr1_mclk = (eClk<<2);
        break;
    case 8:
        clkgen->reg_ckg_sr2_mclk = (eClk<<2);
        break;
    case 12:
        clkgen->reg_ckg_sr3_mclk = (eClk<<2);
        break;
    default:
        pr_err("[VIF] Invalid VIF channel for MCLK.");
        break;
    }
}

void HalVif_EnableParallelIF(VIF_CHANNEL_e ch,u8 en)
{
    infinity2_reg_ispsc_vif *vif = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];
    switch(ch)
    {
    case 0:
    case 4:
    case 8:
    case 12:
        vif->reg_vif_ch0_sensor_sw_rstz = en?1:0;//low active
        break;
    default:
        pr_err("[VIF] Invalid VIF channel for parallel interface.");
        break;
    }
}

void HalVif_GetVifInfo(VIF_CHANNEL_e ch,VifInfo_t *info)
{
	infinity2_reg_ispsc_vif *vif = (infinity2_reg_ispsc_vif *)g_VIFReg[ch];

	if((ch & 0x01) == 0){
	    info->uReceiveWidth = vif->reg_vif_ch0_total_pix_cnt;
        info->uReceiveHeight = vif->reg_vif_ch0_total_line_cnt;
	}else{
        info->uReceiveWidth = vif->reg_vif_ch1_total_pix_cnt;
        info->uReceiveHeight = vif->reg_vif_ch1_total_line_cnt;
	}
}

void HalVif_GlobalInit(void)
{
    VIF_CHANNEL_e ch=VIF_CHANNEL_0;
    for(ch=VIF_CHANNEL_0;ch<VIF_CHANNEL_NUM;++ch)
    {
        HalVif_SensorChannelEnable(ch,VIF_DISABLE);
    }
}

u16 HalVif_FrameStartIrqFinalStatus(void)
{
    VdmaHandle_t *pstHnd = (VdmaHandle_t*) &gstHandle;

    return pstHnd->irq->reg_irq_final_status2;

}


void HalVif_FrameStartIrqMask(u16 ch_bitmask, u8 on_off)
{
    VdmaHandle_t *pstHnd = (VdmaHandle_t*) &gstHandle;

    if (on_off) {
        pstHnd->irq->reg_c_irq_mask2 |= ch_bitmask;
    } else {
        pstHnd->irq->reg_c_irq_mask2 &= (~ch_bitmask);
    }

}
void HalVif_FrameStartIrqClr(u16 ch_bitmask)
{
    VdmaHandle_t *pstHnd = (VdmaHandle_t*) &gstHandle;

    pstHnd->irq->reg_c_irq_clr2 |= ch_bitmask;
    pstHnd->irq->reg_c_irq_clr2 &= (~ch_bitmask);

}
