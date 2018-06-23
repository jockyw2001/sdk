#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>


#include "mi_vdf.h"
#include "../MI/inc/mi_sys.h"
#include "../MI/inc/mi_common.h"


#define VIF_CHN0_ENABLE      1
#define VIF_CHN0_MD0_ENABLE  1   //VDFCHN = 0
#define VIF_CHN0_MD1_ENABLE  0   //VDFCHN = 1
#define VIF_CHN0_OD0_ENABLE  1   //VDFCHN = 2
#define VIF_CHN0_OD1_ENABLE  0   //VDFCHN = 3

#define VIF_CHN1_ENABLE      1
#define VIF_CHN1_OD0_ENABLE  1   //VDFCHN = 10
#define VIF_CHN1_OD1_ENABLE  0   //VDFCHN = 11
#define VIF_CHN1_MD0_ENABLE  0   //VDFCHN = 12
#define VIF_CHN1_MD1_ENABLE  1   //VDFCHN = 13

#define VIF_CHN2_ENABLE      1
#define VIF_CHN2_MD0_ENABLE  1   //VDFCHN = 20
#define VIF_CHN2_MD1_ENABLE  0   //VDFCHN = 21
#define VIF_CHN2_OD0_ENABLE  0   //VDFCHN = 22
#define VIF_CHN2_OD1_ENABLE  1   //VDFCHN = 23

#define VIF_CHN3_ENABLE      1
#define VIF_CHN3_OD0_ENABLE  1   //VDFCHN = 30
#define VIF_CHN3_OD1_ENABLE  0   //VDFCHN = 31
#define VIF_CHN3_MD0_ENABLE  1   //VDFCHN = 32
#define VIF_CHN3_MD1_ENABLE  0   //VDFCHN = 33


#define RAW_W		    320
#define RAW_H		    180
#define MD_DIV_W		12
#define MD_DIV_H		10
#define MD1_DIV_W		16
#define MD1_DIV_H		12
#define OD_DIV_W		3
#define OD_DIV_H		3
#define OD1_DIV_W		2
#define OD1_DIV_H		2


VDF_CHN_ATTR_S stAttr[0x10] = { 0 };
MI_VDF_CHANNEL VdfChn[0x10] = { 0 };


MI_U8 g_exit = 0;



int vdf_set_MD_attr(VDF_CHN_ATTR_S* pstAttr)
{
	pstAttr->enWorkMode = VDF_WORK_MODE_MD;
	pstAttr->unAttr.stMdAttr.enClrType = 1;
	pstAttr->unAttr.stMdAttr.u8SrcChnNum = 0;
	pstAttr->unAttr.stMdAttr.u32MdBufNum = 16;
	pstAttr->unAttr.stMdAttr.u32VDFIntvl = 5; 

	pstAttr->unAttr.stMdAttr.stMDObjCfg.u16Lt_x = 0;
	pstAttr->unAttr.stMdAttr.stMDObjCfg.u16Lt_y = 0;
	pstAttr->unAttr.stMdAttr.stMDObjCfg.u16Rb_x = RAW_W - 1;
	pstAttr->unAttr.stMdAttr.stMDObjCfg.u16Rb_y = RAW_H - 1;
	pstAttr->unAttr.stMdAttr.stMDObjCfg.u16ImgW = RAW_W;
	pstAttr->unAttr.stMdAttr.stMDObjCfg.u16ImgH = RAW_H;
	
	pstAttr->unAttr.stMdAttr.stMDRgnSet.u16W_div = MD_DIV_W;
	pstAttr->unAttr.stMdAttr.stMDRgnSet.u16H_div = MD_DIV_H;
	pstAttr->unAttr.stMdAttr.stMDRgnSet.u32Enable = 0xFFFFFFFF;
	pstAttr->unAttr.stMdAttr.stMDRgnSet.u8Col = 0;
	pstAttr->unAttr.stMdAttr.stMDRgnSet.u8Row = 0;

	pstAttr->unAttr.stMdAttr.stMDParamsIn.enable = 1;
	pstAttr->unAttr.stMdAttr.stMDParamsIn.size_perct_thd_min= 2;
	pstAttr->unAttr.stMdAttr.stMDParamsIn.size_perct_thd_max= 100;
	pstAttr->unAttr.stMdAttr.stMDParamsIn.sensitivity = 70;
	pstAttr->unAttr.stMdAttr.stMDParamsIn.learn_rate = 2000;

	return 0;
}


int vdf_set_OD_attr(VDF_CHN_ATTR_S* pstAttr)
{
    pstAttr->enWorkMode = VDF_WORK_MODE_OD;
    pstAttr->unAttr.stOdAttr.enClrType = 1;
    pstAttr->unAttr.stOdAttr.u8SrcChnNum = 0;
    pstAttr->unAttr.stOdAttr.u32OdBufNum = 16;
    pstAttr->unAttr.stOdAttr.u32VDFIntvl = 5; 

    pstAttr->unAttr.stOdAttr.stODObjCfg.u16Lt_x = 0;
    pstAttr->unAttr.stOdAttr.stODObjCfg.u16Lt_y = 0;
    pstAttr->unAttr.stOdAttr.stODObjCfg.u16Rb_x = RAW_W - 1;
    pstAttr->unAttr.stOdAttr.stODObjCfg.u16Rb_y = RAW_H - 1;
    pstAttr->unAttr.stOdAttr.stODObjCfg.u16ImgW = RAW_W;
    pstAttr->unAttr.stOdAttr.stODObjCfg.u16ImgH = RAW_H;
    
    pstAttr->unAttr.stOdAttr.stODRgnSet.u16W_div = 3; // fill with endiv
    pstAttr->unAttr.stOdAttr.stODRgnSet.u16H_div = 3; // fill with endiv
    pstAttr->unAttr.stOdAttr.stODRgnSet.u32Enable = 0xFFFFFFFF;
    pstAttr->unAttr.stOdAttr.stODRgnSet.u8Col = 0;
    pstAttr->unAttr.stOdAttr.stODRgnSet.u8Row = 0;

    pstAttr->unAttr.stOdAttr.stODParamsIn.endiv = OD_WINDOW_3X3;
    pstAttr->unAttr.stOdAttr.stODParamsIn.s32Thd_tamper = 3;
    pstAttr->unAttr.stOdAttr.stODParamsIn.s32Tamper_blk_thd= 1;
    pstAttr->unAttr.stOdAttr.stODParamsIn.s32Min_duration = 15;
    pstAttr->unAttr.stOdAttr.stODParamsIn.s32Alpha = 2;
	pstAttr->unAttr.stOdAttr.stODParamsIn.s32M = 120;

	return 0;
}

void* vdf_test_loop(void *argu)
{
	char ch = '0';

	while(1)
	{
	    sleep(1);

        ch = getchar();

        if(('q' == ch) || ('Q' == ch))
        {
            g_exit = 1;
        }
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int i, j;
    pthread_t pThread;
    S32 width = RAW_W;
    S32 height = RAW_H;
    VI_CHN chn = 5;

	g_exit = 0;

	pthread_create(&pThread, NULL, vdf_test_loop, NULL);
	pthread_setname_np(pThread, "vdf_test_loop");


//----------------- 1st: do system initial ------------------

//----------------- 2nd: do vdf initial ------------------
    MI_U32 u32VDFVersion[32];

    MI_VDF_Init();
    usleep(10*1000);

//============ VIF-chn=0 =============
#if (VIF_CHN0_ENABLE)  
#if (VIF_CHN0_MD0_ENABLE)
    // initial VDF_HDL(MD)=0
    VdfChn[0] = 0;
	vdf_set_MD_attr(&stAttr[0]);

    MI_VDF_CreateChn(VdfChn[0], &stAttr[0]);
	
    for (i = 0; i < MD_DIV_W; i++)
	{
		for (j = 0; j < MD_DIV_H; j++)
		{
            MI_VDF_EnableChn(VdfChn[0], i, j, 1);
        }
    }
#endif //(VIF_CHN0_MD0_ENABLE)

#if (VIF_CHN0_MD1_ENABLE)
	VdfChn[1] = 1;
	vdf_set_MD_attr(&stAttr[1]);
	stAttr[1].unAttr.stMdAttr.u8SrcChnNum = 0;
    stAttr[1].unAttr.stMdAttr.u32MdBufNum = 13;
    stAttr[1].unAttr.stMdAttr.u32VDFIntvl = 5; 
    
    stAttr[1].unAttr.stMdAttr.stMDRgnSet.u16W_div = MD1_DIV_W;
    stAttr[1].unAttr.stMdAttr.stMDRgnSet.u16H_div = MD1_DIV_H;
    stAttr[1].unAttr.stMdAttr.stMDRgnSet.u32Enable = 0xFFFFFFFF;
    stAttr[1].unAttr.stMdAttr.stMDRgnSet.u8Col = 0;
    stAttr[1].unAttr.stMdAttr.stMDRgnSet.u8Row = 0;

    stAttr[1].unAttr.stMdAttr.stMDParamsIn.enable = 1;
    stAttr[1].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_min= 3;
    stAttr[1].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_max= 100;
    stAttr[1].unAttr.stMdAttr.stMDParamsIn.sensitivity = 80;
    stAttr[1].unAttr.stMdAttr.stMDParamsIn.learn_rate = 2000;

	MI_VDF_CreateChn(VdfChn[1], &stAttr[1]);

	for (i = 0; i < MD1_DIV_W; i++)
	{
		for (j = 0; j < MD1_DIV_H; j++)
		{
			MI_VDF_EnableChn(VdfChn[1], i, j, 1);
		}
	}
#endif //(VIF_CHN0_MD1_ENABLE)

#if (VIF_CHN0_OD0_ENABLE)
		VdfChn[2] = 2;
		vdf_set_OD_attr(&stAttr[2]);
		stAttr[2].enWorkMode = VDF_WORK_MODE_OD;
		stAttr[2].unAttr.stOdAttr.u8SrcChnNum = 0;
		stAttr[2].unAttr.stOdAttr.u32OdBufNum = 10;
		stAttr[2].unAttr.stOdAttr.u32VDFIntvl = 5; 
		
		stAttr[2].unAttr.stOdAttr.stODRgnSet.u16W_div = 3; // fill with endiv
		stAttr[2].unAttr.stOdAttr.stODRgnSet.u16H_div = 3; // fill with endiv
		stAttr[2].unAttr.stOdAttr.stODRgnSet.u32Enable = 0xFFFFFFFF;
		stAttr[2].unAttr.stOdAttr.stODRgnSet.u8Col = 0;
		stAttr[2].unAttr.stOdAttr.stODRgnSet.u8Row = 0;
	
		stAttr[2].unAttr.stOdAttr.stODParamsIn.endiv = OD_WINDOW_3X3;
		stAttr[2].unAttr.stOdAttr.stODParamsIn.s32Thd_tamper = 2;
		stAttr[2].unAttr.stOdAttr.stODParamsIn.s32Tamper_blk_thd= 1;
		stAttr[2].unAttr.stOdAttr.stODParamsIn.s32Min_duration = 15;
		stAttr[2].unAttr.stOdAttr.stODParamsIn.s32Alpha = 2;
		stAttr[2].unAttr.stOdAttr.stODParamsIn.s32M = 120;
	
		MI_VDF_CreateChn(VdfChn[2], &stAttr[2]);
	
		for (i = 0; i < OD_DIV_W; i++)
		{
			for (j = 0; j < OD_DIV_H; j++)
			{
				MI_VDF_EnableChn(VdfChn[2], i, j, 1);
			}
		}
#endif //(VIF_CHN0_OD0_ENABLE)

#if (VIF_CHN0_OD1_ENABLE)
		VdfChn[3] = 3;
		vdf_set_OD_attr(&stAttr[3]);
		stAttr[3].enWorkMode = VDF_WORK_MODE_OD;
		stAttr[3].unAttr.stOdAttr.u8SrcChnNum = 0;
		stAttr[3].unAttr.stOdAttr.u32OdBufNum = 8;
		stAttr[3].unAttr.stOdAttr.u32VDFIntvl = 5; 
		
		stAttr[3].unAttr.stOdAttr.stODRgnSet.u16W_div = 2; // fill with endiv
		stAttr[3].unAttr.stOdAttr.stODRgnSet.u16H_div = 2; // fill with endiv
		stAttr[3].unAttr.stOdAttr.stODRgnSet.u32Enable = 0xFFFFFFFF;
		stAttr[3].unAttr.stOdAttr.stODRgnSet.u8Col = 0;
		stAttr[3].unAttr.stOdAttr.stODRgnSet.u8Row = 0;
	
		stAttr[3].unAttr.stOdAttr.stODParamsIn.endiv = OD_WINDOW_2X2;
		stAttr[3].unAttr.stOdAttr.stODParamsIn.s32Thd_tamper = 2;
		stAttr[3].unAttr.stOdAttr.stODParamsIn.s32Tamper_blk_thd= 1;
		stAttr[3].unAttr.stOdAttr.stODParamsIn.s32Min_duration = 15;
		stAttr[3].unAttr.stOdAttr.stODParamsIn.s32Alpha = 2;
		stAttr[2].unAttr.stOdAttr.stODParamsIn.s32M = 120;
	
		MI_VDF_CreateChn(VdfChn[3], &stAttr[3]);
	
		for (i = 0; i < OD1_DIV_W; i++)
		{
			for (j = 0; j < OD1_DIV_H; j++)
			{
				MI_VDF_EnableChn(VdfChn[3], i, j, 1);
			}
		}
#endif //(VIF_CHN0_OD1_ENABLE)
#endif //(VIF_CHN0_ENABLE)

#if (VIF_CHN1_ENABLE)
#if (VIF_CHN1_OD0_ENABLE)
	VdfChn[4] = 10;
	vdf_set_OD_attr(&stAttr[4]);
	stAttr[4].enWorkMode = VDF_WORK_MODE_OD;
	stAttr[4].unAttr.stOdAttr.u8SrcChnNum = 1;
	stAttr[4].unAttr.stOdAttr.u32OdBufNum = 4;
	stAttr[4].unAttr.stOdAttr.u32VDFIntvl = 5; 
	
	stAttr[4].unAttr.stOdAttr.stODRgnSet.u16W_div = 3; // fill with endiv
	stAttr[4].unAttr.stOdAttr.stODRgnSet.u16H_div = 3; // fill with endiv
	stAttr[4].unAttr.stOdAttr.stODRgnSet.u32Enable = 0xFFFFFFFF;
	stAttr[4].unAttr.stOdAttr.stODRgnSet.u8Col = 0;
	stAttr[4].unAttr.stOdAttr.stODRgnSet.u8Row = 0;

	stAttr[4].unAttr.stOdAttr.stODParamsIn.endiv = OD_WINDOW_3X3;
	stAttr[4].unAttr.stOdAttr.stODParamsIn.s32Thd_tamper = 2;
	stAttr[4].unAttr.stOdAttr.stODParamsIn.s32Tamper_blk_thd= 1;
	stAttr[4].unAttr.stOdAttr.stODParamsIn.s32Min_duration = 15;
	stAttr[4].unAttr.stOdAttr.stODParamsIn.s32Alpha = 2;
	stAttr[4].unAttr.stOdAttr.stODParamsIn.s32M = 120;

	MI_VDF_CreateChn(VdfChn[4], &stAttr[4]);

	for (i = 0; i < OD_DIV_W; i++)
	{
		for (j = 0; j < OD_DIV_H; j++)
		{
            MI_VDF_EnableChn(VdfChn[4], i, j, 1);
        }
    }
#endif //(VIF_CHN1_OD0_ENABLE)

#if (VIF_CHN1_OD1_ENABLE)
	VdfChn[5] = 11;
	vdf_set_OD_attr(&stAttr[5]);
	stAttr[5].enWorkMode = VDF_WORK_MODE_OD;
	stAttr[5].unAttr.stOdAttr.u8SrcChnNum = 1;
	stAttr[5].unAttr.stOdAttr.u32OdBufNum = 5;
	stAttr[5].unAttr.stOdAttr.u32VDFIntvl = 5; 
	
	stAttr[5].unAttr.stOdAttr.stODRgnSet.u16W_div = 2; // fill with endiv
	stAttr[5].unAttr.stOdAttr.stODRgnSet.u16H_div = 2; // fill with endiv
	stAttr[5].unAttr.stOdAttr.stODRgnSet.u32Enable = 0xFFFFFFFF;
	stAttr[5].unAttr.stOdAttr.stODRgnSet.u8Col = 0;
	stAttr[5].unAttr.stOdAttr.stODRgnSet.u8Row = 0;

	stAttr[5].unAttr.stOdAttr.stODParamsIn.endiv = OD_WINDOW_2X2;
	stAttr[5].unAttr.stOdAttr.stODParamsIn.s32Thd_tamper = 2;
	stAttr[5].unAttr.stOdAttr.stODParamsIn.s32Tamper_blk_thd= 1;
	stAttr[5].unAttr.stOdAttr.stODParamsIn.s32Min_duration = 15;
	stAttr[5].unAttr.stOdAttr.stODParamsIn.s32Alpha = 2;
	stAttr[5].unAttr.stOdAttr.stODParamsIn.s32M = 120;

	MI_VDF_CreateChn(VdfChn[5], &stAttr[5]);

	for (i = 0; i < OD1_DIV_W; i++)
	{
		for (j = 0; j < OD1_DIV_H; j++)
		{
            MI_VDF_EnableChn(VdfChn[5], i, j, 1);
        }
    }
#endif //(VIF_CHN1_OD1_ENABLE)

#if (VIF_CHN1_MD0_ENABLE)
		VdfChn[6] = 12;
		vdf_set_MD_attr(&stAttr[6]);
		stAttr[6].unAttr.stMdAttr.u8SrcChnNum = 1;
		stAttr[6].unAttr.stMdAttr.u32MdBufNum = 6;
		stAttr[6].unAttr.stMdAttr.u32VDFIntvl = 5; 
		
		stAttr[6].unAttr.stMdAttr.stMDRgnSet.u16W_div = MD_DIV_W;
		stAttr[6].unAttr.stMdAttr.stMDRgnSet.u16H_div = MD_DIV_H;
		stAttr[6].unAttr.stMdAttr.stMDRgnSet.u32Enable = 0xFFFFFFFF;
		stAttr[6].unAttr.stMdAttr.stMDRgnSet.u8Col = 0;
		stAttr[6].unAttr.stMdAttr.stMDRgnSet.u8Row = 0;
	
		stAttr[6].unAttr.stMdAttr.stMDParamsIn.enable = 1;
		stAttr[6].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_min= 3;
		stAttr[6].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_max= 100;
		stAttr[6].unAttr.stMdAttr.stMDParamsIn.sensitivity = 80;
		stAttr[6].unAttr.stMdAttr.stMDParamsIn.learn_rate = 2000;
	
		MI_VDF_CreateChn(VdfChn[6], &stAttr[6]);
	
		for (i = 0; i < MD_DIV_W; i++)
		{
			for (j = 0; j < MD_DIV_H; j++)
			{
				MI_VDF_EnableChn(VdfChn[6], i, j, 1);
			}
		}
#endif //(VIF_CHN1_MD0_ENABLE)

#if (VIF_CHN1_MD1_ENABLE)
		VdfChn[7] = 13;
		vdf_set_MD_attr(&stAttr[7]);
		stAttr[7].unAttr.stMdAttr.u8SrcChnNum = 1;
		stAttr[7].unAttr.stMdAttr.u32MdBufNum = 7;
		stAttr[7].unAttr.stMdAttr.u32VDFIntvl = 5; 
		
		stAttr[7].unAttr.stMdAttr.stMDRgnSet.u16W_div = MD1_DIV_W;
		stAttr[7].unAttr.stMdAttr.stMDRgnSet.u16H_div = MD1_DIV_H;
		stAttr[7].unAttr.stMdAttr.stMDRgnSet.u32Enable = 0xFFFFFFFF;
		stAttr[7].unAttr.stMdAttr.stMDRgnSet.u8Col = 0;
		stAttr[7].unAttr.stMdAttr.stMDRgnSet.u8Row = 0;
	
		stAttr[7].unAttr.stMdAttr.stMDParamsIn.enable = 1;
		stAttr[7].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_min= 3;
		stAttr[7].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_max= 100;
		stAttr[7].unAttr.stMdAttr.stMDParamsIn.sensitivity = 80;
		stAttr[7].unAttr.stMdAttr.stMDParamsIn.learn_rate = 2000;
	
		MI_VDF_CreateChn(VdfChn[7], &stAttr[7]);
	
		for (i = 0; i < MD1_DIV_W; i++)
		{
			for (j = 0; j < MD1_DIV_H; j++)
			{
				MI_VDF_EnableChn(VdfChn[7], i, j, 1);
			}
		}
#endif //(VIF_CHN1_MD1_ENABLE)
#endif //(VIF_CHN1_ENABLE)

#if (VIF_CHN2_ENABLE)
#if (VIF_CHN2_MD0_ENABLE)
    VdfChn[8] = 20;
	vdf_set_MD_attr(&stAttr[8]);
    stAttr[8].unAttr.stMdAttr.u8SrcChnNum = 2;
    stAttr[8].unAttr.stMdAttr.u32MdBufNum = 8;
    stAttr[8].unAttr.stMdAttr.u32VDFIntvl = 5; 
    
    stAttr[8].unAttr.stMdAttr.stMDRgnSet.u16W_div = MD_DIV_W;
    stAttr[8].unAttr.stMdAttr.stMDRgnSet.u16H_div = MD_DIV_H;
    stAttr[8].unAttr.stMdAttr.stMDRgnSet.u32Enable = 0xFFFFFFFF;
    stAttr[8].unAttr.stMdAttr.stMDRgnSet.u8Col = 0;
    stAttr[8].unAttr.stMdAttr.stMDRgnSet.u8Row = 0;

    stAttr[8].unAttr.stMdAttr.stMDParamsIn.enable = 1;
    stAttr[8].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_min= 3;
    stAttr[8].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_max= 100;
    stAttr[8].unAttr.stMdAttr.stMDParamsIn.sensitivity = 80;
    stAttr[8].unAttr.stMdAttr.stMDParamsIn.learn_rate = 2000;

    MI_VDF_CreateChn(VdfChn[8], &stAttr[8]);
	
    for (i = 0; i < MD_DIV_W; i++)
	{
		for (j = 0; j < MD_DIV_H; j++)
		{
            MI_VDF_EnableChn(VdfChn[8], i, j, 1);
        }
    }
#endif //(VIF_CHN2_MD0_ENABLE)

#if (VIF_CHN2_MD1_ENABLE)
    VdfChn[9] = 21;
	vdf_set_MD_attr(&stAttr[9]);
    stAttr[8].unAttr.stMdAttr.u8SrcChnNum = 2;
    stAttr[8].unAttr.stMdAttr.u32MdBufNum = 9;
    stAttr[8].unAttr.stMdAttr.u32VDFIntvl = 5; 
    
    stAttr[8].unAttr.stMdAttr.stMDRgnSet.u16W_div = MD1_DIV_W;
    stAttr[8].unAttr.stMdAttr.stMDRgnSet.u16H_div = MD1_DIV_H;
    stAttr[8].unAttr.stMdAttr.stMDRgnSet.u32Enable = 0xFFFFFFFF;
    stAttr[8].unAttr.stMdAttr.stMDRgnSet.u8Col = 0;
    stAttr[8].unAttr.stMdAttr.stMDRgnSet.u8Row = 0;

    stAttr[8].unAttr.stMdAttr.stMDParamsIn.enable = 1;
    stAttr[8].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_min= 3;
    stAttr[8].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_max= 100;
    stAttr[8].unAttr.stMdAttr.stMDParamsIn.sensitivity = 80;
    stAttr[8].unAttr.stMdAttr.stMDParamsIn.learn_rate = 2000;

    MI_VDF_CreateChn(VdfChn[9], &stAttr[9]);
	
    for (i = 0; i < MD1_DIV_W; i++)
	{
		for (j = 0; j < MD1_DIV_H; j++)
		{
            MI_VDF_EnableChn(VdfChn[9], i, j, 1);
        }
    }
#endif //(VIF_CHN2_MD1_ENABLE)

#if (VIF_CHN2_OD0_ENABLE)
		VdfChn[10] = 22;
		vdf_set_OD_attr(&stAttr[10]);
		stAttr[10].enWorkMode = VDF_WORK_MODE_OD;
		stAttr[10].unAttr.stOdAttr.u8SrcChnNum = 2;
		stAttr[10].unAttr.stOdAttr.u32OdBufNum = 10;
		stAttr[10].unAttr.stOdAttr.u32VDFIntvl = 5; 
		
		stAttr[10].unAttr.stOdAttr.stODRgnSet.u16W_div = 3; // fill with endiv
		stAttr[10].unAttr.stOdAttr.stODRgnSet.u16H_div = 3; // fill with endiv
		stAttr[10].unAttr.stOdAttr.stODRgnSet.u32Enable = 0xFFFFFFFF;
		stAttr[10].unAttr.stOdAttr.stODRgnSet.u8Col = 0;
		stAttr[10].unAttr.stOdAttr.stODRgnSet.u8Row = 0;
	
		stAttr[10].unAttr.stOdAttr.stODParamsIn.endiv = OD_WINDOW_3X3;
		stAttr[10].unAttr.stOdAttr.stODParamsIn.s32Thd_tamper = 2;
		stAttr[10].unAttr.stOdAttr.stODParamsIn.s32Tamper_blk_thd= 1;
		stAttr[10].unAttr.stOdAttr.stODParamsIn.s32Min_duration = 15;
		stAttr[10].unAttr.stOdAttr.stODParamsIn.s32Alpha = 2;
		stAttr[10].unAttr.stOdAttr.stODParamsIn.s32M = 120;
	
		MI_VDF_CreateChn(VdfChn[10], &stAttr[10]);
	
		for (i = 0; i < OD_DIV_W; i++)
		{
			for (j = 0; j < OD_DIV_H; j++)
			{
				MI_VDF_EnableChn(VdfChn[10], i, j, 1);
			}
		}
#endif //(VIF_CHN2_OD0_ENABLE)

#if (VIF_CHN2_OD1_ENABLE)
		VdfChn[11] = 23;
		vdf_set_OD_attr(&stAttr[11]);
		stAttr[11].enWorkMode = VDF_WORK_MODE_OD;
		stAttr[11].unAttr.stOdAttr.u8SrcChnNum = 2;
		stAttr[11].unAttr.stOdAttr.u32OdBufNum = 11;
		stAttr[11].unAttr.stOdAttr.u32VDFIntvl = 5; 
		
		stAttr[11].unAttr.stOdAttr.stODRgnSet.u16W_div = 2; // fill with endiv
		stAttr[11].unAttr.stOdAttr.stODRgnSet.u16H_div = 2; // fill with endiv
		stAttr[11].unAttr.stOdAttr.stODRgnSet.u32Enable = 0xFFFFFFFF;
		stAttr[11].unAttr.stOdAttr.stODRgnSet.u8Col = 0;
		stAttr[11].unAttr.stOdAttr.stODRgnSet.u8Row = 0;
	
		stAttr[11].unAttr.stOdAttr.stODParamsIn.endiv = OD_WINDOW_2X2;
		stAttr[11].unAttr.stOdAttr.stODParamsIn.s32Thd_tamper = 2;
		stAttr[11].unAttr.stOdAttr.stODParamsIn.s32Tamper_blk_thd= 1;
		stAttr[11].unAttr.stOdAttr.stODParamsIn.s32Min_duration = 15;
		stAttr[11].unAttr.stOdAttr.stODParamsIn.s32Alpha = 2;
		stAttr[11].unAttr.stOdAttr.stODParamsIn.s32M = 120;
	
		MI_VDF_CreateChn(VdfChn[11], &stAttr[11]);
	
		for (i = 0; i < OD1_DIV_W; i++)
		{
			for (j = 0; j < OD1_DIV_H; j++)
			{
				MI_VDF_EnableChn(VdfChn[11], i, j, 1);
			}
		}
#endif //(VIF_CHN2_OD1_ENABLE)
#endif //(VIF_CHN2_ENABLE)

#if (VIF_CHN3_ENABLE)
#if (VIF_CHN3_OD0_ENABLE)
	VdfChn[12] = 30;
	vdf_set_OD_attr(&stAttr[12]);
	stAttr[12].enWorkMode = VDF_WORK_MODE_OD;
	stAttr[12].unAttr.stOdAttr.u8SrcChnNum = 3;
	stAttr[12].unAttr.stOdAttr.u32OdBufNum = 2;
	stAttr[12].unAttr.stOdAttr.u32VDFIntvl = 5; 
	
	stAttr[12].unAttr.stOdAttr.stODRgnSet.u16W_div = 3;
	stAttr[12].unAttr.stOdAttr.stODRgnSet.u16H_div = 3;
	stAttr[12].unAttr.stOdAttr.stODRgnSet.u32Enable = 0xFFFFFFFF;
	stAttr[12].unAttr.stOdAttr.stODRgnSet.u8Col = 0;
	stAttr[12].unAttr.stOdAttr.stODRgnSet.u8Row = 0;

	stAttr[12].unAttr.stOdAttr.stODParamsIn.endiv = OD_WINDOW_3X3;
	stAttr[12].unAttr.stOdAttr.stODParamsIn.s32Thd_tamper = 3;
	stAttr[12].unAttr.stOdAttr.stODParamsIn.s32Tamper_blk_thd= 1;
	stAttr[12].unAttr.stOdAttr.stODParamsIn.s32Min_duration = 15;
	stAttr[12].unAttr.stOdAttr.stODParamsIn.s32Alpha = 2;
	stAttr[12].unAttr.stOdAttr.stODParamsIn.s32M = 120;

	MI_VDF_CreateChn(VdfChn[12], &stAttr[12]);

	for (i = 0; i < OD_DIV_W; i++)
	{
		for (j = 0; j < OD_DIV_W; j++)
		{
            MI_VDF_EnableChn(VdfChn[12], i, j, 1);
        }
    }
#endif //(VIF_CHN3_OD0_ENABLE)

#if (VIF_CHN3_OD1_ENABLE)
	VdfChn[13] = 31;
	vdf_set_OD_attr(&stAttr[13]);
	stAttr[13].enWorkMode = VDF_WORK_MODE_OD;
	stAttr[13].unAttr.stOdAttr.u8SrcChnNum = 3;
	stAttr[13].unAttr.stOdAttr.u32OdBufNum = 3;
	stAttr[13].unAttr.stOdAttr.u32VDFIntvl = 5; 
	
	stAttr[13].unAttr.stOdAttr.stODRgnSet.u16W_div = 2;
	stAttr[13].unAttr.stOdAttr.stODRgnSet.u16H_div = 2;
	stAttr[13].unAttr.stOdAttr.stODRgnSet.u32Enable = 0xFFFFFFFF;
	stAttr[13].unAttr.stOdAttr.stODRgnSet.u8Col = 0;
	stAttr[13].unAttr.stOdAttr.stODRgnSet.u8Row = 0;

	stAttr[13].unAttr.stOdAttr.stODParamsIn.endiv = OD_WINDOW_2X2;
	stAttr[13].unAttr.stOdAttr.stODParamsIn.s32Thd_tamper = 3;
	stAttr[13].unAttr.stOdAttr.stODParamsIn.s32Tamper_blk_thd= 1;
	stAttr[13].unAttr.stOdAttr.stODParamsIn.s32Min_duration = 15;
	stAttr[13].unAttr.stOdAttr.stODParamsIn.s32Alpha = 2;
	stAttr[13].unAttr.stOdAttr.stODParamsIn.s32M = 120;

	MI_VDF_CreateChn(VdfChn[13], &stAttr[13]);

	for (i = 0; i < OD1_DIV_W; i++)
	{
		for (j = 0; j < OD1_DIV_W; j++)
		{
            MI_VDF_EnableChn(VdfChn[13], i, j, 1);
        }
    }
#endif //(VIF_CHN3_OD1_ENABLE)

#if (VIF_CHN3_MD0_ENABLE)
    VdfChn[14] = 32;
	vdf_set_MD_attr(&stAttr[14]);
    stAttr[14].unAttr.stMdAttr.u8SrcChnNum = 3;
    stAttr[14].unAttr.stMdAttr.u32MdBufNum = 4;
    stAttr[14].unAttr.stMdAttr.u32VDFIntvl = 5; 
    
    stAttr[14].unAttr.stMdAttr.stMDRgnSet.u16W_div = MD_DIV_W;
    stAttr[14].unAttr.stMdAttr.stMDRgnSet.u16H_div = MD_DIV_H;
    stAttr[14].unAttr.stMdAttr.stMDRgnSet.u32Enable = 0xFFFFFFFF;
    stAttr[14].unAttr.stMdAttr.stMDRgnSet.u8Col = 0;
    stAttr[14].unAttr.stMdAttr.stMDRgnSet.u8Row = 0;

    stAttr[14].unAttr.stMdAttr.stMDParamsIn.enable = 1;
    stAttr[14].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_min= 3;
    stAttr[14].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_max= 100;
    stAttr[14].unAttr.stMdAttr.stMDParamsIn.sensitivity = 80;
    stAttr[14].unAttr.stMdAttr.stMDParamsIn.learn_rate = 2000;

    MI_VDF_CreateChn(VdfChn[14], &stAttr[14]);
	
    for (i = 0; i < MD_DIV_W; i++)
	{
		for (j = 0; j < MD_DIV_H; j++)
		{
            MI_VDF_EnableChn(VdfChn[14], i, j, 1);
        }
    }
#endif //(VIF_CHN3_MD0_ENABLE)

#if (VIF_CHN3_MD1_ENABLE)
    VdfChn[15] = 33;
	vdf_set_MD_attr(&stAttr[15]);
    stAttr[15].unAttr.stMdAttr.u8SrcChnNum = 3;
    stAttr[15].unAttr.stMdAttr.u32MdBufNum = 5;
    stAttr[15].unAttr.stMdAttr.u32VDFIntvl = 5; 
    
    stAttr[15].unAttr.stMdAttr.stMDRgnSet.u16W_div = MD1_DIV_W;
    stAttr[15].unAttr.stMdAttr.stMDRgnSet.u16H_div = MD1_DIV_H;
    stAttr[15].unAttr.stMdAttr.stMDRgnSet.u32Enable = 0xFFFFFFFF;
    stAttr[15].unAttr.stMdAttr.stMDRgnSet.u8Col = 0;
    stAttr[15].unAttr.stMdAttr.stMDRgnSet.u8Row = 0;

    stAttr[15].unAttr.stMdAttr.stMDParamsIn.enable = 1;
    stAttr[15].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_min= 3;
    stAttr[15].unAttr.stMdAttr.stMDParamsIn.size_perct_thd_max= 100;
    stAttr[15].unAttr.stMdAttr.stMDParamsIn.sensitivity = 80;
    stAttr[15].unAttr.stMdAttr.stMDParamsIn.learn_rate = 2000;

    MI_VDF_CreateChn(VdfChn[15], &stAttr[15]);
	
    for (i = 0; i < MD1_DIV_W; i++)
	{
		for (j = 0; j < MD1_DIV_H; j++)
		{
            MI_VDF_EnableChn(VdfChn[15], i, j, 1);
        }
    }
#endif //(VIF_CHN3_MD1_ENABLE)
#endif //(VIF_CHN3_ENABLE)


    MI_VDF_GetLibVersion(VdfChn[0], u32VDFVersion);

	MI_VDF_Run(VDF_WORK_MODE_MD);
	MI_VDF_Run(VDF_WORK_MODE_OD);

    sleep(1);

//----------------- 3th: get MD/OD result ------------------
	while(0 == g_exit)
	{
		MI_S32 ret = 0;
        MI_VDF_RESULT_S stVdfResult = { 0 };
        
	    usleep(100*1000);

#if (VIF_CHN0_ENABLE) // VIF-Chn=0
#if (VIF_CHN0_MD0_ENABLE)
        memset(&stVdfResult, 0x00, sizeof(MI_VDF_RESULT_S));
        stVdfResult.enWorkMode = VDF_WORK_MODE_MD;
		ret = MI_VDF_GetResult(VdfChn[0], &stVdfResult, 0);
        if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
        {
            printf("[%s:%d][HDL=00] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get MD-Rst data:\n", __func__,__LINE__,
                                stVdfResult.unVdfResult.stMdResult.u64Pts, \
                                stVdfResult.enWorkMode, \
                                stVdfResult.unVdfResult.stMdResult.u8Enable, \
                                stVdfResult.unVdfResult.stMdResult.u8W_div, \
                                stVdfResult.unVdfResult.stMdResult.u8H_div);
			#if 1
			printf("    0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx  ",	
								stVdfResult.unVdfResult.stMdResult.u64Md_result[0],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[1],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[2],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[3],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[4],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[5],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[6],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[7]);
			printf("0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx\n",
								stVdfResult.unVdfResult.stMdResult.u64Md_result[8],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[9],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[10],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[11],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[12],
                    			stVdfResult.unVdfResult.stMdResult.u64Md_result[13],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[14],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[15]),

			#else
            printf("    0x%016llx  0x%016llx  0x%016llx  0x%016llx\n",  
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[0],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[1],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[2],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[3]);
            printf("    0x%016llx  0x%016llx  0x%016llx  0x%016llx\n",  
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[4],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[5],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[6],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[7]);
            printf("    0x%016llx  0x%016llx  0x%016llx  0x%016llx\n",  
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[8],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[9],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[10],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[11]);
			#endif
            MI_VDF_PutResult(VdfChn[0], &stVdfResult);
        }
#endif //(VIF_CHN0_MD0_ENABLE)

#if (VIF_CHN0_MD1_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(MI_VDF_RESULT_S));
		stVdfResult.enWorkMode = VDF_WORK_MODE_MD;
		ret = MI_VDF_GetResult(VdfChn[1], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=01] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get MD-Rst data:\n", __func__,__LINE__,
								stVdfResult.unVdfResult.stMdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stMdResult.u8Enable, \
								stVdfResult.unVdfResult.stMdResult.u8W_div, \
								stVdfResult.unVdfResult.stMdResult.u8H_div);
			printf("    0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx  ",	
								stVdfResult.unVdfResult.stMdResult.u64Md_result[0],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[1],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[2],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[3],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[4],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[5],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[6],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[7]);
			printf("0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx\n",
								stVdfResult.unVdfResult.stMdResult.u64Md_result[8],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[9],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[10],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[11],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[12],
                    			stVdfResult.unVdfResult.stMdResult.u64Md_result[13],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[14],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[15]),
			MI_VDF_PutResult(VdfChn[1], &stVdfResult);
		}
#endif //(VIF_CHN0_MD1_ENABLE)

#if (VIF_CHN0_OD0_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(stVdfResult));
		stVdfResult.enWorkMode = VDF_WORK_MODE_OD;
		ret = MI_VDF_GetResult(VdfChn[2], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=02] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get OD-Rst data: ", __func__,__LINE__,
								stVdfResult.unVdfResult.stOdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stOdResult.u8Enable, \
								stVdfResult.unVdfResult.stOdResult.u8W_div, \
								stVdfResult.unVdfResult.stOdResult.u8H_div);
			printf("{%u %u %u  %u %u %u  %u %u %u}\n", 
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][2]);
			MI_VDF_PutResult(VdfChn[2], &stVdfResult);
		}
#endif //(VIF_CHN0_OD0_ENABLE)

#if (VIF_CHN0_OD1_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(stVdfResult));
		stVdfResult.enWorkMode = VDF_WORK_MODE_OD;
		ret = MI_VDF_GetResult(VdfChn[3], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=03] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get OD-Rst data: ", __func__,__LINE__,
								stVdfResult.unVdfResult.stOdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stOdResult.u8Enable, \
								stVdfResult.unVdfResult.stOdResult.u8W_div, \
								stVdfResult.unVdfResult.stOdResult.u8H_div);
			printf("{%u %u %u  %u %u %u  %u %u %u}\n\n", 
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][2]);
			MI_VDF_PutResult(VdfChn[3], &stVdfResult);
		}
#endif //(VIF_CHN0_OD0_ENABLE)
#endif //(VIF_CHN0_ENABLE)

#if (VIF_CHN1_ENABLE)
#if (VIF_CHN1_OD0_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(stVdfResult));
		stVdfResult.enWorkMode = VDF_WORK_MODE_OD;
		ret = MI_VDF_GetResult(VdfChn[4], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=10] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get OD-Rst data: ", __func__,__LINE__,
								stVdfResult.unVdfResult.stOdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stOdResult.u8Enable, \
								stVdfResult.unVdfResult.stOdResult.u8W_div, \
								stVdfResult.unVdfResult.stOdResult.u8H_div);
			printf("{%u %u %u  %u %u %u  %u %u %u}\n",	
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][2]);
			MI_VDF_PutResult(VdfChn[4], &stVdfResult);
		}
#endif //(VIF_CHN1_OD0_ENABLE)

#if (VIF_CHN1_OD1_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(stVdfResult));
		stVdfResult.enWorkMode = VDF_WORK_MODE_OD;
		ret = MI_VDF_GetResult(VdfChn[5], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=11] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get OD-Rst data: ", __func__,__LINE__,
								stVdfResult.unVdfResult.stOdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stOdResult.u8Enable, \
								stVdfResult.unVdfResult.stOdResult.u8W_div, \
								stVdfResult.unVdfResult.stOdResult.u8H_div);
			printf("{%u %u %u  %u %u %u  %u %u %u}\n",	
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][2]);
			MI_VDF_PutResult(VdfChn[5], &stVdfResult);
		}
#endif //(VIF_CHN1_OD1_ENABLE)

#if (VIF_CHN1_MD0_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(MI_VDF_RESULT_S));
		stVdfResult.enWorkMode = VDF_WORK_MODE_MD;
		ret = MI_VDF_GetResult(VdfChn[6], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=12] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get MD-Rst data: ", __func__,__LINE__,
								stVdfResult.unVdfResult.stMdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stMdResult.u8Enable, \
								stVdfResult.unVdfResult.stMdResult.u8W_div, \
								stVdfResult.unVdfResult.stMdResult.u8H_div);
			printf("    0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx  ",	
								stVdfResult.unVdfResult.stMdResult.u64Md_result[0],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[1],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[2],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[3],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[4],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[5],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[6],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[7]);
			printf("0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx\n",
								stVdfResult.unVdfResult.stMdResult.u64Md_result[8],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[9],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[10],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[11],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[12],
                    			stVdfResult.unVdfResult.stMdResult.u64Md_result[13],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[14],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[15]),

			MI_VDF_PutResult(VdfChn[6], &stVdfResult);
		}
#endif //(VIF_CHN1_MD0_ENABLE)

#if (VIF_CHN1_MD1_ENABLE)
        memset(&stVdfResult, 0x00, sizeof(stVdfResult));
        stVdfResult.enWorkMode = VDF_WORK_MODE_MD;
		ret = MI_VDF_GetResult(VdfChn[7], &stVdfResult, 0);
        if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
        {
            printf("[%s:%d][HDL=13] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get MD-Rst data:\n", __func__,__LINE__,
                                stVdfResult.unVdfResult.stMdResult.u64Pts, \
                                stVdfResult.enWorkMode, \
                                stVdfResult.unVdfResult.stMdResult.u8Enable, \
                                stVdfResult.unVdfResult.stMdResult.u8W_div, \
                                stVdfResult.unVdfResult.stMdResult.u8H_div);
			printf("    0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx  ",	
								stVdfResult.unVdfResult.stMdResult.u64Md_result[0],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[1],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[2],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[3],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[4],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[5],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[6],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[7]);
			printf("0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx\n\n",
								stVdfResult.unVdfResult.stMdResult.u64Md_result[8],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[9],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[10],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[11],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[12],
                    			stVdfResult.unVdfResult.stMdResult.u64Md_result[13],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[14],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[15]),

            MI_VDF_PutResult(VdfChn[7], &stVdfResult);
        }
#endif //(VIF_CHN1_MD1_ENABLE)
#endif //VIF_CHN1_ENABLE

#if (VIF_CHN2_ENABLE)
#if (VIF_CHN2_MD0_ENABLE)
        memset(&stVdfResult, 0x00, sizeof(stVdfResult));
        stVdfResult.enWorkMode = VDF_WORK_MODE_MD;
		ret = MI_VDF_GetResult(VdfChn[8], &stVdfResult, 0);
        if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
        {
            printf("[%s:%d][HDL=20] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get MD-Rst data:\n", __func__,__LINE__,
                                stVdfResult.unVdfResult.stMdResult.u64Pts, \
                                stVdfResult.enWorkMode, \
                                stVdfResult.unVdfResult.stMdResult.u8Enable, \
                                stVdfResult.unVdfResult.stMdResult.u8W_div, \
                                stVdfResult.unVdfResult.stMdResult.u8H_div);
			printf("    0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx  ",	
								stVdfResult.unVdfResult.stMdResult.u64Md_result[0],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[1],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[2],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[3],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[4],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[5],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[6],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[7]);
			printf("0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx\n",
								stVdfResult.unVdfResult.stMdResult.u64Md_result[8],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[9],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[10],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[11],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[12],
                    			stVdfResult.unVdfResult.stMdResult.u64Md_result[13],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[14],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[15]),

            MI_VDF_PutResult(VdfChn[8], &stVdfResult);
        }
#endif //(VIF_CHN2_MD0_ENABLE)

#if (VIF_CHN2_MD1_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(stVdfResult));
		stVdfResult.enWorkMode = VDF_WORK_MODE_MD;
		ret = MI_VDF_GetResult(VdfChn[9], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=21] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get MD-Rst data:\n", __func__,__LINE__,
								stVdfResult.unVdfResult.stMdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stMdResult.u8Enable, \
								stVdfResult.unVdfResult.stMdResult.u8W_div, \
								stVdfResult.unVdfResult.stMdResult.u8H_div);
			printf("    0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx  ",	
								stVdfResult.unVdfResult.stMdResult.u64Md_result[0],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[1],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[2],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[3],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[4],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[5],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[6],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[7]);
			printf("0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx\n",
								stVdfResult.unVdfResult.stMdResult.u64Md_result[8],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[9],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[10],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[11],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[12],
                    			stVdfResult.unVdfResult.stMdResult.u64Md_result[13],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[14],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[15]),

			MI_VDF_PutResult(VdfChn[9], &stVdfResult);
		}
#endif //(VIF_CHN2_MD1_ENABLE)

#if (VIF_CHN2_OD0_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(stVdfResult));
		stVdfResult.enWorkMode = VDF_WORK_MODE_OD;
		ret = MI_VDF_GetResult(VdfChn[10], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=22] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get OD-Rst data: ", __func__,__LINE__,
								stVdfResult.unVdfResult.stOdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stOdResult.u8Enable, \
								stVdfResult.unVdfResult.stOdResult.u8W_div, \
								stVdfResult.unVdfResult.stOdResult.u8H_div);
			printf("{%u %u %u  %u %u %u  %u %u %u}\n", 
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][2]);
			MI_VDF_PutResult(VdfChn[10], &stVdfResult);
		}	   
#endif //(VIF_CHN2_OD0_ENABLE)

#if (VIF_CHN2_OD1_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(stVdfResult));
		stVdfResult.enWorkMode = VDF_WORK_MODE_OD;
		ret = MI_VDF_GetResult(VdfChn[11], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=23] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get OD-Rst data: ", __func__,__LINE__,
								stVdfResult.unVdfResult.stOdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stOdResult.u8Enable, \
								stVdfResult.unVdfResult.stOdResult.u8W_div, \
								stVdfResult.unVdfResult.stOdResult.u8H_div);
			printf("{%u %u %u  %u %u %u  %u %u %u}\n\n", 
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][2]);
			MI_VDF_PutResult(VdfChn[11], &stVdfResult);
		}	   
#endif //(VIF_CHN2_OD1_ENABLE)
#endif //(VIF_CHN2_ENABLE)

#if (VIF_CHN3_ENABLE)
#if (VIF_CHN3_OD0_ENABLE)
        memset(&stVdfResult, 0x00, sizeof(stVdfResult));
        stVdfResult.enWorkMode = VDF_WORK_MODE_OD;
		ret = MI_VDF_GetResult(VdfChn[12], &stVdfResult, 0);
        if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
        {
            printf("[%s:%d][HDL=30] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get OD-Rst data: ", __func__,__LINE__,
                                stVdfResult.unVdfResult.stOdResult.u64Pts, \
                                stVdfResult.enWorkMode, \
                                stVdfResult.unVdfResult.stOdResult.u8Enable, \
                                stVdfResult.unVdfResult.stOdResult.u8W_div, \
                                stVdfResult.unVdfResult.stOdResult.u8H_div);
            printf("{%u %u %u  %u %u %u  %u %u %u}\n",	
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][2]);
            MI_VDF_PutResult(VdfChn[12], &stVdfResult);
        }      
#endif //(VIF_CHN3_OD0_ENABLE)
		
#if (VIF_CHN3_OD1_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(stVdfResult));
		stVdfResult.enWorkMode = VDF_WORK_MODE_OD;
		ret = MI_VDF_GetResult(VdfChn[13], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=31] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get OD-Rst data: ", __func__,__LINE__,
								stVdfResult.unVdfResult.stOdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stOdResult.u8Enable, \
								stVdfResult.unVdfResult.stOdResult.u8W_div, \
								stVdfResult.unVdfResult.stOdResult.u8H_div);
			printf("{%u %u %u  %u %u %u  %u %u %u}\n", 
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[0][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[1][2],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][0],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][1],
								stVdfResult.unVdfResult.stOdResult.u8RgnAlarm[2][2]);
			MI_VDF_PutResult(VdfChn[13], &stVdfResult);
		}	   
#endif //(VIF_CHN3_OD1_ENABLE)

#if (VIF_CHN3_MD0_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(stVdfResult));
		stVdfResult.enWorkMode = VDF_WORK_MODE_MD;
		ret = MI_VDF_GetResult(VdfChn[14], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=32] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get MD-Rst data:\n", __func__,__LINE__,
								stVdfResult.unVdfResult.stMdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stMdResult.u8Enable, \
								stVdfResult.unVdfResult.stMdResult.u8W_div, \
								stVdfResult.unVdfResult.stMdResult.u8H_div);
			printf("    0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx  ",	
								stVdfResult.unVdfResult.stMdResult.u64Md_result[0],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[1],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[2],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[3],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[4],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[5],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[6],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[7]);
			printf("0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx\n",
								stVdfResult.unVdfResult.stMdResult.u64Md_result[8],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[9],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[10],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[11],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[12],
                    			stVdfResult.unVdfResult.stMdResult.u64Md_result[13],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[14],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[15]),

			MI_VDF_PutResult(VdfChn[14], &stVdfResult);
		}
#endif //(VIF_CHN3_MD0_ENABLE)

#if (VIF_CHN3_MD1_ENABLE)
		memset(&stVdfResult, 0x00, sizeof(stVdfResult));
		stVdfResult.enWorkMode = VDF_WORK_MODE_MD;
		ret = MI_VDF_GetResult(VdfChn[15], &stVdfResult, 0);
		if((0 == ret) && (1 == stVdfResult.unVdfResult.stMdResult.u8Enable))
		{
			printf("[%s:%d][HDL=33] pts=0x%llx [WorkMode=%d, Enable=%d, (%d, %d)] Get MD-Rst data:\n", __func__,__LINE__,
								stVdfResult.unVdfResult.stMdResult.u64Pts, \
								stVdfResult.enWorkMode, \
								stVdfResult.unVdfResult.stMdResult.u8Enable, \
								stVdfResult.unVdfResult.stMdResult.u8W_div, \
								stVdfResult.unVdfResult.stMdResult.u8H_div);
			printf("    0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx  ",	
								stVdfResult.unVdfResult.stMdResult.u64Md_result[0],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[1],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[2],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[3],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[4],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[5],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[6],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[7]);
			printf("0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx 0x%04hx\n\n",
								stVdfResult.unVdfResult.stMdResult.u64Md_result[8],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[9],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[10],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[11],
								stVdfResult.unVdfResult.stMdResult.u64Md_result[12],
                    			stVdfResult.unVdfResult.stMdResult.u64Md_result[13],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[14],
                                stVdfResult.unVdfResult.stMdResult.u64Md_result[15]),

			MI_VDF_PutResult(VdfChn[9], &stVdfResult);
		}
#endif //(VIF_CHN3_MD1_ENABLE)
#endif //(VIF_CHN3_ENABLE)
	}


//----------------- 4th: do vdf un-initial ------------------
    for (i = 0; i < MD_DIV_W; i++)
	{
		for (j = 0; j < MD_DIV_H; j++)
		{
		    VDF_CHN_ATTR_S stAttr_tmp = { 0 };
            
            MI_VDF_EnableChn(VdfChn[0], i, j, 0);

            memset(&stAttr_tmp, 0x00, sizeof(stAttr_tmp));
            MI_VDF_GetChnAttr(VdfChn[0], &stAttr_tmp);
		}
	}
        
    MI_VDF_DestroyChn(VdfChn[0]);
    MI_VDF_Stop(VDF_WORK_MODE_MD);
    MI_VDF_Uninit();

    printf("vdf example exit\n");
    return 0;
}
