//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    drvAVD.c
/// @brief  AVD Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
#include "drvDMD_INTERN_DVBC.h"
#include "drvDMD_INTERN_DVBC_v2.h"
#include "UFO.h"
//#include <string.h>
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#else
#include <string.h>
#include <stdio.h>
#include <math.h>
#endif
#include "MsCommon.h"
#include "MsVersion.h"
#include "MsOS.h"
// Internal Definition
//#include "regCHIP.h"
//#include "regAVD.h"
//#include "mapi_tuner.h"
#include "drvSYS.h"
#include "drvDMD_VD_MBX.h"
#include "halDMD_INTERN_DVBC.h"
#include "halDMD_INTERN_common.h"
#include "../../include/drvSAR.h"  // for Utopia2
#include "utopia.h"   
#include "utopia_dapi.h"
#include "ULog.h"
#include "../../include/drvDMD_INTERN_DVBC_v2.h"  // for Utopia2
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------
#ifdef MS_DEBUG
#define DMD_DBG(x)          (x)
#else
#define DMD_DBG(x)          //(x)
#endif

static    void* ppDVBCInstant = NULL;
static MS_U32 u32DVBCopen = 0;
static MS_U8 u8DVBCUtopiaOpen = 0;   //for SetStillImagePara is earlier called than Init  

static DMD_DVBC_InitData   AgcSsi_Para;

#if defined(CHIP_K6LITE)||defined(CHIP_KAISER)
static DMD_DVBC_InitData   AgcSsi_Para_0;
static DMD_DVBC_InitData   AgcSsi_Para_1;
static MS_U8               previous_demod_index=0xff;
#endif

static float intern_dvb_c_qam_ref[] = {3.0, 0.0, 0.0, 0.0, 0.0, 80.0}; //16q,32q,64q,128q,256q, and others

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
#ifdef UFO_DEMOD_DVBC_SUPPORT_DMD_INT
#define DMD_EVT_MASK          0x0F
#define DMD_EVT_MCU_INT       0x01
#define DMD_EVT_TASK_STACK_SIZE 4096
static MS_U8  u8StackBuffer[DMD_EVT_TASK_STACK_SIZE];
static MS_S32 _s32DmdEventId = -1;
static MS_S32 _s32DmdEventTaskId = -1;
static MS_U32 u32Events;

static fpIntCallBack fpCB = NULL;

static MS_U16 u16DMD_DVBC_AutoSymbol_Timeout = 10000;
static MS_U16  u16DMD_DVBC_FixSymbol_AutoQam_Timeout=2000;
static MS_U16  u16DMD_DVBC_FixSymbol_FixQam_Timeout=1000;


//DEMOD A: Timeout setting
static MS_S32 _s32DmdTimerId_FIRST_TIMEOUT_A = -1;//TR_A
static MS_S32 _s32DmdTimerId_SECOND_TIMEOUT_A = -1; // FEC_A
static MS_U32 DMD_TIMER_FIRST_TIMEOUT_A =    500 ;//TBD  //TR_A
static MS_U32 DMD_TIMER_SECOND_TIMEOUT_A=     2000; //TBD  // FEC_A

//DEMOD B: Timeout setting
static MS_S32 _s32DmdTimerId_FIRST_TIMEOUT_B = -1;//TR_A
static MS_S32 _s32DmdTimerId_SECOND_TIMEOUT_B = -1; // FEC_A
static MS_U32 DMD_TIMER_FIRST_TIMEOUT_B =   500 ;//TBD  //TR_A
static MS_U32 DMD_TIMER_SECOND_TIMEOUT_B=     2000; //TBD  // FEC_A

MS_BOOL MDrv_DMD_DVBC_Reg_INT_CB(fpIntCallBack fpCBReg)
{
     fpCB = fpCBReg;
    return TRUE;
}

//============================================================
//Function: MDrv_DMD_DVBC_Timer_CB  
// (1) demod index:   demod A    0x00
//                                 demod B   0x01                    
// (2) lock status:    0x00   unknow
//                              0x01   lock
//                              0x02   unlock
//                              0x03   checking
//============================================================
void MDrv_DMD_DVBC_Timer_CB(MS_U32 u32StTimer, MS_U32 u32TimerID)
{    
	MS_U8  status=0x00;
	MS_U8 demod_index=0x00;

	DMD_DVBC_INT_TYPE  lock_status=0x00;
	DMD_DVBC_LOCK_STATUS  TR_ever_lock_A=0;
	DMD_DVBC_LOCK_STATUS  TR_ever_lock_B=0;
	
	DVBC_GET_INFO_TR_EVER_LOCK_IN pin;
	DVBC_GET_INFO_TR_EVER_LOCK_OUT pout;



	if ((u32TimerID == _s32DmdTimerId_FIRST_TIMEOUT_A) || (u32TimerID ==_s32DmdTimerId_SECOND_TIMEOUT_A))
		demod_index=0x00;
	else if  ((u32TimerID ==_s32DmdTimerId_FIRST_TIMEOUT_B) || (u32TimerID ==_s32DmdTimerId_SECOND_TIMEOUT_B))
		demod_index=0x01;

	
	pin.demod_no=demod_index;
	DMD_DBG(printf("===>   The demod index: %d \n",pin.demod_no));
	pin.eType=DMD_DVBC_GETLOCK_TR_EVER_LOCK;

	
	if (demod_index==0x00)
	{
		pout.peLockStatus=&TR_ever_lock_A;
	}
	else if (demod_index==0x01)
	{
		pout.peLockStatus=&TR_ever_lock_B;
	}
	pout.ret=false;



	if (demod_index==0x00)
	{
		if (u32TimerID == _s32DmdTimerId_FIRST_TIMEOUT_A)
		{

			MDrv_DMD_DVBC_Get_Info_Generalization(GET_INFO_TR_EVER_LOCK, &pin, &pout);
			
			if (TR_ever_lock_A != DMD_DVBC_UNLOCK)
			{
				lock_status=0x03; //checking
				printf("===>  [019] Time: 500ms;   Demod A is checking......... \n");
			}
			else
			{
				lock_status=0x02; //unlock
				printf("===>  [020] Time: 500ms;   Demod A is unlock......... \n");
				MsOS_StopTimer (_s32DmdTimerId_SECOND_TIMEOUT_A);			
			}	
		}
		else if (u32TimerID == _s32DmdTimerId_SECOND_TIMEOUT_A)
		{
			lock_status=0x02; //unlock
			printf("===>  [021] Time: (**) %d ms;   Demod A is unlock......... \n",DMD_TIMER_SECOND_TIMEOUT_A);
		}
	}

	else if (demod_index==0x01)
	{

		if (u32TimerID == _s32DmdTimerId_FIRST_TIMEOUT_B) 
		{

			MDrv_DMD_DVBC_Get_Info_Generalization(GET_INFO_TR_EVER_LOCK, &pin, &pout);

			if (TR_ever_lock_B != DMD_DVBC_UNLOCK)
			{
				lock_status=0x03; //checking
				printf("===>  [024] Time: 500ms;   Demod B is checking......... \n");		
			}
			else
			{
				lock_status=0x02;  //unlock
				printf("===>  [025] Time: 500ms;   Demod B is unlock......... \n");
				MsOS_StopTimer (_s32DmdTimerId_SECOND_TIMEOUT_B);
			}	
		}
		else if (u32TimerID == _s32DmdTimerId_SECOND_TIMEOUT_B)
		{
			lock_status=0x02; //unlock
			printf("===>  [026] Time: %d ms;   Demod B is unlock......... \n",DMD_TIMER_SECOND_TIMEOUT_B);

		}
	
	}
	status= ((demod_index << 4)+ lock_status);

	printf("===>  [015] Result: Demod [%d]   lock status: %d  \n",demod_index,lock_status);

	if(fpCB!=NULL)
	{
     		fpCB(status);
	}
}

static void _mdrv_dmd_dvbc_event_task(MS_U32 argc, void *argv)
{
   MS_U8 u8IntType;
  do
  {
     MsOS_WaitEvent(_s32DmdEventId, DMD_EVT_MCU_INT,&u32Events, E_OR_CLEAR, MSOS_WAIT_FOREVER);
     INTERN_DVBC_DEMOD_INTERRUPT_MONITOR(&u8IntType);

//START: Stop the timer	 
	if ((u8IntType>>4)==0x00)
	{
		if(_s32DmdTimerId_FIRST_TIMEOUT_A > 0)
	     	{
			MsOS_StopTimer (_s32DmdTimerId_FIRST_TIMEOUT_A);
			
		}
		
		if(_s32DmdTimerId_SECOND_TIMEOUT_A>0)
		{
			MsOS_StopTimer (_s32DmdTimerId_SECOND_TIMEOUT_A);
			
		}

	}
	else if ((u8IntType>>4)==0x01)
	{
		if(_s32DmdTimerId_FIRST_TIMEOUT_B > 0)
	     	{
			MsOS_StopTimer (_s32DmdTimerId_FIRST_TIMEOUT_B);
			
		}
		
		if(_s32DmdTimerId_SECOND_TIMEOUT_B>0)
		{
			MsOS_StopTimer (_s32DmdTimerId_SECOND_TIMEOUT_B);
		
		}

	}

//END: Stop the timer	 
 
     //if(_sDMD_DVBC_InitData.fpCB != NULL)
     if(fpCB!=NULL)
     {
       fpCB((DMD_DVBC_INT_TYPE)u8IntType);
     }
  }while(1);
}

static void _mdrv_dmd_dvbc_cb(InterruptNum irq)
{
    MsOS_ClearEvent(_s32DmdEventId, DMD_EVT_MASK); 
    MsOS_SetEvent(_s32DmdEventId, DMD_EVT_MCU_INT);
    MsOS_EnableInterrupt(E_INT_FIQ_DMDMCU2HK);  
}
#endif







#ifndef MSOS_TYPE_LINUX
#if 1
static float _LogApproxTableX[80] =
{ 1.00, 1.30, 1.69, 2.20, 2.86, 3.71, 4.83, 6.27, 8.16, 10.60, 13.79,
  17.92, 23.30, 30.29, 39.37, 51.19, 66.54, 86.50, 112.46, 146.19,
  190.05, 247.06, 321.18, 417.54, 542.80, 705.64, 917.33, 1192.53,
  1550.29, 2015.38, 2620.00, 3405.99, 4427.79, 5756.13, 7482.97,
  9727.86, 12646.22, 16440.08, 21372.11, 27783.74, 36118.86,
  46954.52, 61040.88, 79353.15, 103159.09, 134106.82, 174338.86,
  226640.52, 294632.68, 383022.48, 497929.22, 647307.99, 841500.39, 1093950.50,
  1422135.65, 1848776.35, 2403409.25, 3124432.03, 4061761.64, 5280290.13,
  6864377.17, 8923690.32, 11600797.42, 15081036.65, 19605347.64, 25486951.94,
  33133037.52, 43072948.77, 55994833.40, 72793283.42, 94631268.45,
  123020648.99, 159926843.68, 207904896.79, 270276365.82, 351359275.57,
  456767058.24, 593797175.72, 771936328.43, 1003517226.96
};

static float _LogApproxTableY[80] =
{ 0.00, 0.11, 0.23, 0.34, 0.46, 0.57, 0.68, 0.80, 0.91, 1.03, 1.14, 1.25,
  1.37, 1.48, 1.60, 1.71, 1.82, 1.94, 2.05, 2.16, 2.28, 2.39, 2.51, 2.62,
  2.73, 2.85, 2.96, 3.08, 3.19, 3.30, 3.42, 3.53, 3.65, 3.76, 3.87, 3.99,
  4.10, 4.22, 4.33, 4.44, 4.56, 4.67, 4.79, 4.90, 5.01, 5.13, 5.24, 5.36,
  5.47, 5.58, 5.70, 5.81, 5.93, 6.04, 6.15, 6.27, 6.04, 6.15, 6.27, 6.38,
  6.49, 6.61, 6.72, 6.84, 6.95, 7.06, 7.18, 7.29, 7.41, 7.52, 7.63, 7.75,
  7.86, 7.98, 8.09, 8.20, 8.32, 8.43, 8.55, 8.66
};

static float Log10Approx(float flt_x)
{
    MS_U8  indx = 0;

    do {
        if (flt_x < _LogApproxTableX[indx])
            break;
        indx++;
    }while (indx < 79);   //stop at indx = 80

    return _LogApproxTableY[indx];
}
#else
static float Log10Approx(float flt_x)
{
    MS_U32       u32_temp = 1;
    MS_U8        indx = 0;

    do {
        u32_temp = u32_temp << 1;
        if (flt_x < (float)u32_temp)
            break;
    }while (++indx < 32);

    // 10*log10(X) ~= 0.3*N, when X ~= 2^N
    return (float)0.3 * indx;
}
#endif
#endif

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
#if defined(CHIP_KAISER)||defined(CHIP_K6LITE)
MS_BOOL MDrv_DMD_DVBC_Dual_Public_Init(MS_U8 u8AGC_Tristate_Ctrl,MS_U8 u8Sar_Channel)
{
   void* pAttribte = NULL;
  DVBC_Dual_Public_Init_PARAM Drv_DVBC_Dual_Public_Init_PARAM;
  Drv_DVBC_Dual_Public_Init_PARAM.u8AGC_Tristate_Ctrl=u8AGC_Tristate_Ctrl;
  Drv_DVBC_Dual_Public_Init_PARAM.u8Sar_Channel=u8Sar_Channel;
  
     if(u8DVBCUtopiaOpen == 0)  // First time open
    {
        if(UtopiaOpen(MODULE_DVBC/*|KERNEL_MODE*/ , &ppDVBCInstant, 0, pAttribte) == UTOPIA_STATUS_SUCCESS)  //kernel space
//if(UtopiaOpen(MODULE_DVBC , &ppDVBCInstant, 0, pAttribte) == UTOPIA_STATUS_SUCCESS)  //user space
        {
            u32DVBCopen = 1;
      //return_val=true;
            //ULOGD("DEMOD","\r\n ======== DVBT Open Successful %x =========", (WORD)u32DVBTopen);
        }
        else
        {
           // ULOGD("DEMOD","\r\n ======== DVBT Open Fail %x =========", (WORD)u32DVBTopen);
       //return_val=false;

          return false;
        }

        u8DVBCUtopiaOpen = 1;
    }
    
   if(u32DVBCopen==1)
        UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_Dual_Public_Init,&Drv_DVBC_Dual_Public_Init_PARAM);
      else
        return false;
  
#ifdef UFO_DEMOD_DVBC_SUPPORT_DMD_INT
    if (_s32DmdEventId < 0)
    {
        _s32DmdEventId = MsOS_CreateEventGroup("DMD_INTERN_DVBC_Event");

		
    #ifdef MS_DEBUG
        if (_s32DmdEventId > 0)
            printf("[%s][%d] Event create ok\n",__FUNCTION__,__LINE__);
        else
            printf("[%s][%d] create failed \n",__FUNCTION__,__LINE__);
#endif
    }

    MsOS_AttachInterrupt(E_INT_FIQ_DMDMCU2HK, _mdrv_dmd_dvbc_cb);

    MsOS_DisableInterrupt(E_INT_FIQ_DMDMCU2HK); 
	
    if(_s32DmdEventTaskId < 0)
    {
       _s32DmdEventTaskId = MsOS_CreateTask(_mdrv_dmd_dvbc_event_task,
                                            0,
                                            E_TASK_PRI_HIGHEST,
                                            TRUE,
                                            u8StackBuffer,
                                            DMD_EVT_TASK_STACK_SIZE,
                                            "DMD_INTERN_DVBC_EVT_TASK");

	   
    #ifdef MS_DEBUG
        if (_s32DmdEventTaskId > 0)
            printf("[%s][%d] Event task create ok\n",__FUNCTION__,__LINE__);
        else
            printf("[%s][%d] create task failed \n",__FUNCTION__,__LINE__);
#endif
    }
#endif    


  
      return Drv_DVBC_Dual_Public_Init_PARAM.ret;
}


MS_BOOL MDrv_DMD_DVBC_Dual_Individual_Init(DMD_DVBC_InitData *pDMD_DVBC_InitData, MS_U32 u32InitDataLen)
{  
  DVBC_Dual_Individual_Init_PARAM Drv_DVBC_Dual_Individual_Init_PARAM;
  DMD_DVBC_InitData_Transform Init_Para_Temp;
  
  
  Init_Para_Temp.u8SarChannel=pDMD_DVBC_InitData->u8SarChannel;
  Init_Para_Temp.u8DMD_DVBC_DSPRegInitExt=pDMD_DVBC_InitData->u8DMD_DVBC_DSPRegInitExt;
  Init_Para_Temp.u8DMD_DVBC_DSPRegInitSize=pDMD_DVBC_InitData->u8DMD_DVBC_DSPRegInitSize;
  Init_Para_Temp.u8DMD_DVBC_InitExt=pDMD_DVBC_InitData->u8DMD_DVBC_InitExt;
  
//bryan add
AgcSsi_Para.pTuner_RfagcSsi=pDMD_DVBC_InitData->pTuner_RfagcSsi;
AgcSsi_Para.u16Tuner_RfagcSsi_Size=pDMD_DVBC_InitData->u16Tuner_RfagcSsi_Size;
AgcSsi_Para.pTuner_IfagcSsi_LoRef=pDMD_DVBC_InitData->pTuner_IfagcSsi_LoRef;
AgcSsi_Para.u16Tuner_IfagcSsi_LoRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcSsi_LoRef_Size;     
AgcSsi_Para.pTuner_IfagcSsi_HiRef=pDMD_DVBC_InitData->pTuner_IfagcSsi_HiRef;
AgcSsi_Para.u16Tuner_IfagcSsi_HiRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcSsi_HiRef_Size;
     
     
  /*
  Init_Para_Temp.pTuner_RfagcSsi=pDMD_DVBC_InitData->pTuner_RfagcSsi;
  Init_Para_Temp.u16Tuner_RfagcSsi_Size=pDMD_DVBC_InitData->u16Tuner_RfagcSsi_Size;
  Init_Para_Temp.pTuner_IfagcSsi_LoRef=pDMD_DVBC_InitData->pTuner_IfagcSsi_LoRef;
  Init_Para_Temp.u16Tuner_IfagcSsi_LoRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcSsi_LoRef_Size;  
  Init_Para_Temp.pTuner_IfagcSsi_HiRef=pDMD_DVBC_InitData->pTuner_IfagcSsi_HiRef;
  Init_Para_Temp.u16Tuner_IfagcSsi_HiRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcSsi_HiRef_Size;
  Init_Para_Temp.pTuner_IfagcErr_LoRef=pDMD_DVBC_InitData->pTuner_IfagcErr_LoRef;
  Init_Para_Temp.u16Tuner_IfagcErr_LoRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcErr_LoRef_Size;
  Init_Para_Temp.pTuner_IfagcSsi_HiRef=pDMD_DVBC_InitData->pTuner_IfagcSsi_HiRef;
  Init_Para_Temp.u16Tuner_IfagcErr_HiRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcErr_HiRef_Size;
  Init_Para_Temp.pSqiCnNordigP1=pDMD_DVBC_InitData->pSqiCnNordigP1;
  Init_Para_Temp.u16SqiCnNordigP1_Size=pDMD_DVBC_InitData->u16SqiCnNordigP1_Size;
  */
  
  Drv_DVBC_Dual_Individual_Init_PARAM.pDMD_DVBC_InitData=&Init_Para_Temp;
  Drv_DVBC_Dual_Individual_Init_PARAM.u32InitDataLen=sizeof(Init_Para_Temp);
  
  if(u32DVBCopen==1)
      UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_Dual_Individual_Init,&Drv_DVBC_Dual_Individual_Init_PARAM);
  else
      return false;

  return Drv_DVBC_Dual_Individual_Init_PARAM.ret;
}
#else
MS_BOOL MDrv_DMD_DVBC_Dual_Individual_Init(DMD_DVBC_InitData *pDMD_DVBC_InitData, MS_U32 u32InitDataLen)
{ 
        ULOGD("DEMOD","Doesn't support DVBC_Dual_Individual_Init function!!!\n");
        return false;
}

MS_BOOL MDrv_DMD_DVBC_Dual_Public_Init(MS_U8 u8AGC_Tristate_Ctrl,MS_U8 u8Sar_Channel)
{
        ULOGD("DEMOD","Doesn't support DVBC_Dual_Public_Init function!!!\n");
        return false;
}
#endif

MS_BOOL MDrv_DMD_DVBC_Init(DMD_DVBC_InitData *pDMD_DVBC_InitData, MS_U32 u32InitDataLen)
{
    void* pAttribte = NULL;

    #ifdef MS_DEBUG
    ULOGD("DEMOD","******check driver layer DVBC init!!*******\n");
    #endif
    DMD_DBG(ULOGD("DEMOD","[drvDMD_INTERN_DVBC.c]MDrv_DMD_DVBC_Init\n"));

    DVBC_Init_PARAM Drv_DVBC_Init_PARAM;
    DMD_DVBC_InitData_Transform Init_Para_Temp;

    Init_Para_Temp.u8SarChannel=pDMD_DVBC_InitData->u8SarChannel;
    Init_Para_Temp.u8DMD_DVBC_DSPRegInitExt=pDMD_DVBC_InitData->u8DMD_DVBC_DSPRegInitExt;
    Init_Para_Temp.u8DMD_DVBC_DSPRegInitSize=pDMD_DVBC_InitData->u8DMD_DVBC_DSPRegInitSize;
    Init_Para_Temp.u8DMD_DVBC_InitExt=pDMD_DVBC_InitData->u8DMD_DVBC_InitExt;
    /*
    Init_Para_Temp.pTuner_RfagcSsi=pDMD_DVBC_InitData->pTuner_RfagcSsi;
    Init_Para_Temp.u16Tuner_RfagcSsi_Size=pDMD_DVBC_InitData->u16Tuner_RfagcSsi_Size;
    Init_Para_Temp.pTuner_IfagcSsi_LoRef=pDMD_DVBC_InitData->pTuner_IfagcSsi_LoRef;
    Init_Para_Temp.u16Tuner_IfagcSsi_LoRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcSsi_LoRef_Size;  
    Init_Para_Temp.pTuner_IfagcSsi_HiRef=pDMD_DVBC_InitData->pTuner_IfagcSsi_HiRef;
    Init_Para_Temp.u16Tuner_IfagcSsi_HiRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcSsi_HiRef_Size;
    Init_Para_Temp.pTuner_IfagcErr_LoRef=pDMD_DVBC_InitData->pTuner_IfagcErr_LoRef;
    Init_Para_Temp.u16Tuner_IfagcErr_LoRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcErr_LoRef_Size;
    Init_Para_Temp.pTuner_IfagcSsi_HiRef=pDMD_DVBC_InitData->pTuner_IfagcSsi_HiRef;
    Init_Para_Temp.u16Tuner_IfagcErr_HiRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcErr_HiRef_Size;
    Init_Para_Temp.pSqiCnNordigP1=pDMD_DVBC_InitData->pSqiCnNordigP1;
    Init_Para_Temp.u16SqiCnNordigP1_Size=pDMD_DVBC_InitData->u16SqiCnNordigP1_Size;
    */

    memcpy (&(AgcSsi_Para), pDMD_DVBC_InitData, sizeof(DMD_DVBC_InitData));
    /*
    AgcSsi_Para.pTuner_RfagcSsi=pDMD_DVBC_InitData->pTuner_RfagcSsi;
    AgcSsi_Para.u16Tuner_RfagcSsi_Size=pDMD_DVBC_InitData->u16Tuner_RfagcSsi_Size;
    AgcSsi_Para.pTuner_IfagcSsi_LoRef=pDMD_DVBC_InitData->pTuner_IfagcSsi_LoRef;
    AgcSsi_Para.u16Tuner_IfagcSsi_LoRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcSsi_LoRef_Size;  
    AgcSsi_Para.pTuner_IfagcSsi_HiRef=pDMD_DVBC_InitData->pTuner_IfagcSsi_HiRef;
    AgcSsi_Para.u16Tuner_IfagcSsi_HiRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcSsi_HiRef_Size;
    AgcSsi_Para.pTuner_IfagcErr_LoRef=pDMD_DVBC_InitData->pTuner_IfagcErr_LoRef;
    AgcSsi_Para.u16Tuner_IfagcErr_LoRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcErr_LoRef_Size;
    AgcSsi_Para.pTuner_IfagcSsi_HiRef=pDMD_DVBC_InitData->pTuner_IfagcSsi_HiRef;
    AgcSsi_Para.u16Tuner_IfagcErr_HiRef_Size=pDMD_DVBC_InitData->u16Tuner_IfagcErr_HiRef_Size;

    AgcSsi_Para.u8SarChannel=pDMD_DVBC_InitData->u8SarChannel;
    AgcSsi_Para.pSqiCnNordigP1=pDMD_DVBC_InitData->pSqiCnNordigP1;
    AgcSsi_Para.u16SqiCnNordigP1_Size=pDMD_DVBC_InitData->u16SqiCnNordigP1_Size;
    AgcSsi_Para.u8DMD_DVBC_DSPRegInitExt=pDMD_DVBC_InitData->u8DMD_DVBC_DSPRegInitExt;
    AgcSsi_Para.u8DMD_DVBC_DSPRegInitSize=pDMD_DVBC_InitData->u8DMD_DVBC_DSPRegInitSize;
    AgcSsi_Para.u8DMD_DVBC_InitExt=pDMD_DVBC_InitData->u8DMD_DVBC_InitExt;
    */

    Drv_DVBC_Init_PARAM.u32InitDataLen=sizeof(Init_Para_Temp);
    Drv_DVBC_Init_PARAM.pDMD_DVBC_InitData=&Init_Para_Temp;
    Drv_DVBC_Init_PARAM.ret=false;    
     
     if(u8DVBCUtopiaOpen == 0)  // First time open
    { 
        if(UtopiaOpen(MODULE_DVBC/*|KERNEL_MODE*/ , &ppDVBCInstant, 0, pAttribte) == UTOPIA_STATUS_SUCCESS)  //kernel space
        //if(UtopiaOpen(MODULE_DVBC , &ppDVBCInstant, 0, pAttribte) == UTOPIA_STATUS_SUCCESS)  //user space
        {
            u32DVBCopen = 1;
            ULOGD("DEMOD","bryan check DVBC utopia open sucessful!!\n");
      //return_val=true;
           // ULOGD("DEMOD","\r\n ======== DVBC Open Successful %x =========", (WORD)u32DVBCopen);
        }
        else
        {
          //  ULOGD("DEMOD","\r\n ======== DVBC Open Fail %x =========", (WORD)u32DVBCopen);
       //return_val=false;
          ULOGD("DEMOD","DVBC utopia open fail!!\n");
          return false;
        }

        u8DVBCUtopiaOpen = 1;
    }
    
   if(u32DVBCopen==1)
        UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_Init,&Drv_DVBC_Init_PARAM);
      else
        return false;

    return Drv_DVBC_Init_PARAM.ret;
}

#ifdef UFO_DEMOD_DVBC_GET_AGC_INFO
MS_BOOL MDrv_DMD_DVBC_GetAGCInfo(MS_U8 u8dbg_mode, MS_U16 *pu16Data)
{
    DVBC_GetAGCInfo_PARAM Drv_DVBC_GET_AGC_INFO;

    Drv_DVBC_GET_AGC_INFO.u8dbg_mode = u8dbg_mode;
    Drv_DVBC_GET_AGC_INFO.pu16Data = pu16Data;
    Drv_DVBC_GET_AGC_INFO.ret = false;

    if(u32DVBCopen==1)
    {
        UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetAGCInfo, &Drv_DVBC_GET_AGC_INFO);
    }
    
    return Drv_DVBC_GET_AGC_INFO.ret;	
}
#endif


MS_BOOL MDrv_DMD_DVBC_Exit(void)
{
    DVBC_EXIT_PARAM_PARAM Drv_DVBC_EXIT_PARAM_PARAM;
    Drv_DVBC_EXIT_PARAM_PARAM.ret=false;
    DMD_DBG(ULOGD("DEMOD","[drvDMD_INTERN_DVBC.c]MDrv_DMD_DVBC_Exit\n"));

       if(u32DVBCopen==1)
        UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_Exit,&Drv_DVBC_EXIT_PARAM_PARAM);
      else
        return false;

    return Drv_DVBC_EXIT_PARAM_PARAM.ret;
}

MS_BOOL MDrv_DMD_DVBC_SetDbgLevel(DMD_DVBC_DbgLv u8DbgLevel)
{
    DVBC_SetDbgLevel_PARAM Drv_DVBC_SetDbgLevel_PARAM;
    Drv_DVBC_SetDbgLevel_PARAM.u8DbgLevel=u8DbgLevel;
    Drv_DVBC_SetDbgLevel_PARAM.ret=false;
  
    DMD_DBG(ULOGD("DEMOD","[drvDMD_INTERN_DVBC.c]MDrv_DMD_DVBC_SetDbgLevel\n"));
      if(u32DVBCopen==1)
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_SetDbgLevel,&Drv_DVBC_SetDbgLevel_PARAM);
      else
        return false;
      
    return Drv_DVBC_SetDbgLevel_PARAM.ret;
}

#ifndef MSOS_TYPE_LINUX_KERNEL
const DMD_DVBC_Info* MDrv_DMD_DVBC_GetInfo(void)
{
  DVBC_GetInfo_PARAM Drv_DVBC_GetInfo_PARAM;
  Drv_DVBC_GetInfo_PARAM.ret_info=NULL;
  DMD_DVBC_MODULATION_TYPE Qam_mode;
  float fSNR;
  MS_U16 u16Quality;
  
  MS_U16 SymbolRate;
  float FreqOff;
  
  if(u32DVBCopen==1)
  {
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetInfo,&Drv_DVBC_GetInfo_PARAM);
    MDrv_DMD_DVBC_GetSNR(&fSNR);
    MDrv_DMD_DVBC_GetSignalQuality(&u16Quality);
    MDrv_DMD_DVBC_GetStatus(&Qam_mode, &SymbolRate, &FreqOff);

  }
  else
  {
  }


  return Drv_DVBC_GetInfo_PARAM.ret_info;

}
#endif

MS_BOOL MDrv_DMD_DVBC_GetLibVer(const MSIF_Version **ppVersion)
{
   DVBC_GetLibVer_PARAM Drv_DVBC_GetLibVer_PARAM;
   Drv_DVBC_GetLibVer_PARAM.ppVersion=ppVersion;
    Drv_DVBC_GetLibVer_PARAM.ret= false;


   if(u32DVBCopen==1)
   {
       UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetLibVer,&Drv_DVBC_GetLibVer_PARAM);
	    Drv_DVBC_GetLibVer_PARAM.ret= true;
   }
   else
   {
       return false;
   }
  
   return Drv_DVBC_GetLibVer_PARAM.ret;
}

MS_BOOL MDrv_DMD_DVBC_GetFWVer(MS_U16 *ver)
{
  DVBC_GetFWVer_PARAM Drv_DVBC_GetFWVer_PARAM;
  Drv_DVBC_GetFWVer_PARAM.ver=ver;
  Drv_DVBC_GetFWVer_PARAM.ret=false;
  
        if(u32DVBCopen==1)
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetFWVer,&Drv_DVBC_GetFWVer_PARAM);
  else
  {
    return false;
  }

       return Drv_DVBC_GetFWVer_PARAM.ret;

}

MS_BOOL MDrv_DMD_DVBC_GetDSPReg(MS_U16 u16Addr, MS_U8 *pu8Data)
{
#if defined(CHIP_K1)||defined(CHIP_KENYA)||defined(CONFIG_UTOPIA_PROC_DBG_SUPPORT)
  DVBC_GetDSPReg_PARAM Drv_DVBC_GetDSPReg_PARAM;
  Drv_DVBC_GetDSPReg_PARAM.u16Addr=u16Addr;
  Drv_DVBC_GetDSPReg_PARAM.pu8Data=pu8Data;
  
   if(u32DVBCopen==1)
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetDSPReg,&Drv_DVBC_GetDSPReg_PARAM);
  else
  {
    return false;
  }

  return Drv_DVBC_GetDSPReg_PARAM.ret;
#else
    ULOGD("DEMOD","Not Support function: MDrv_DMD_DVBC_GetDSPReg\n");
  return FALSE;
#endif
}

MS_BOOL MDrv_DMD_DVBC_SetDSPReg(MS_U16 u16Addr, MS_U8 pu8Data)
{
#if defined(CHIP_K1)||defined(CHIP_KENYA)
  DVBC_SetDSPReg_PARAM Drv_DVBC_SetDSPReg_PARAM;

  Drv_DVBC_SetDSPReg_PARAM.pu8Data=pu8Data;
  Drv_DVBC_SetDSPReg_PARAM.u16Addr=u16Addr;
  
  if(u32DVBCopen==1)
  {
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_SetDSPReg,&Drv_DVBC_SetDSPReg_PARAM);
  }
  else
  {
    return false;
  }

    return Drv_DVBC_SetDSPReg_PARAM.ret;
#else
    ULOGD("DEMOD","Not Support function: MDrv_DMD_DVBC_SetDSPReg\n");
  return FALSE;
#endif
}


MS_BOOL MDrv_DMD_DVBC_GetReg(MS_U16 u16Addr, MS_U8 *pu8Data)
{
  DVBC_GetReg_PARAM Drv_DVBC_GetReg_PARAM;
  Drv_DVBC_GetReg_PARAM.u16Addr=u16Addr;
  Drv_DVBC_GetReg_PARAM.pu8Data=pu8Data;
  Drv_DVBC_GetReg_PARAM.ret=false;

  if(u32DVBCopen==1)
    {
      UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetReg,&Drv_DVBC_GetReg_PARAM);
   }
    else
    {
      return false;
    }

    return Drv_DVBC_GetReg_PARAM.ret;
  
}

MS_BOOL MDrv_DMD_DVBC_SetReg(MS_U16 u16Addr, MS_U8 u8Data)
{
  DVBC_SetReg_PARAM Drv_DVBC_SetReg_PARAM;
  Drv_DVBC_SetReg_PARAM.u16Addr=u16Addr;
  Drv_DVBC_SetReg_PARAM.u8Data=u8Data;
  Drv_DVBC_SetReg_PARAM.ret=false; 

  if(u32DVBCopen==1)
    {
      UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_SetReg,&Drv_DVBC_SetReg_PARAM);
   }
    else
    {
      return false;
    }

    return Drv_DVBC_SetReg_PARAM.ret;
}

MS_BOOL MDrv_DMD_DVBC_SetSerialControl(MS_BOOL bEnable)
{
      DVBC_SetSerialControl_PARAM Drv_DVBC_SetSerialControl_PARAM;
      Drv_DVBC_SetSerialControl_PARAM.bEnable=bEnable;
  Drv_DVBC_SetSerialControl_PARAM.ret=false;

  if(u32DVBCopen==1)
    {
      UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_SetSerialControl,&Drv_DVBC_SetSerialControl_PARAM);
   }
    else
    {
      return false;
    }    

    
    return Drv_DVBC_SetSerialControl_PARAM.ret;
}

MS_U32 MDrv_DMD_DVBC_GetConfig(DMD_DVBC_InitData *pDMD_DVBC_InitData)  
{
	if (!u32DVBCopen) return FALSE;
	
       memcpy (pDMD_DVBC_InitData, &(AgcSsi_Para), sizeof(DMD_DVBC_InitData));	
    
       return UTOPIA_STATUS_SUCCESS;
}

MS_BOOL MDrv_DMD_DVBC_SetConfig(MS_U16 u16SymbolRate, DMD_DVBC_MODULATION_TYPE eQamMode, MS_U32 u32IFFreq, MS_BOOL bSpecInv, MS_BOOL bSerialTS)
{
  #ifdef MS_DEBUG
  ULOGD("DEMOD","check driver layer DVBC SetConfig!!\n");
  #endif
  
   return MDrv_DMD_DVBC_SetConfig_symbol_rate_list(u16SymbolRate, eQamMode, u32IFFreq, bSpecInv, bSerialTS, NULL, 0);
}

MS_BOOL MDrv_DMD_DVBC_SetConfig_symbol_rate_list(MS_U16 u16SymbolRate, DMD_DVBC_MODULATION_TYPE eQamMode, MS_U32 u32IFFreq, MS_BOOL bSpecInv, MS_BOOL bSerialTS, MS_U16 *pu16_symbol_rate_list,MS_U8 u8_symbol_rate_list_num)
{ 
  #ifdef MS_DEBUG
  ULOGD("DEMOD","check driver layer DVBC SetConfig_symbol_rate_list!!\n");
  #endif

  DVBC_SetConfig_Symbol_rate_list_PARAM Drv_DVBC_SetConfig_Symbol_rate_list_PARAM;
  Drv_DVBC_SetConfig_Symbol_rate_list_PARAM.u16SymbolRate=u16SymbolRate;
  Drv_DVBC_SetConfig_Symbol_rate_list_PARAM.eQamMode=eQamMode;
  Drv_DVBC_SetConfig_Symbol_rate_list_PARAM.u32IFFreq=u32IFFreq;
  Drv_DVBC_SetConfig_Symbol_rate_list_PARAM.bSpecInv=bSpecInv;
  Drv_DVBC_SetConfig_Symbol_rate_list_PARAM.bSerialTS=bSerialTS;
  Drv_DVBC_SetConfig_Symbol_rate_list_PARAM.pu16_symbol_rate_list=pu16_symbol_rate_list;
  Drv_DVBC_SetConfig_Symbol_rate_list_PARAM.u8_symbol_rate_list_num=u8_symbol_rate_list_num;
  Drv_DVBC_SetConfig_Symbol_rate_list_PARAM.ret=false;  
  
//update the second timeout
#if defined(CHIP_K6LITE)||defined(CHIP_KAISER)

  if (previous_demod_index==0x00)
  {
	  if (u16SymbolRate == 0)
	  {
		  DMD_TIMER_SECOND_TIMEOUT_A = u16DMD_DVBC_AutoSymbol_Timeout;
	  }
	  else
	  {
		  DMD_TIMER_SECOND_TIMEOUT_A = (eQamMode == DMD_DVBC_QAMAUTO) ? u16DMD_DVBC_FixSymbol_AutoQam_Timeout : u16DMD_DVBC_FixSymbol_FixQam_Timeout;
	  }
  }
  else if(previous_demod_index==0x01)
  {
	  if (u16SymbolRate == 0)
	  {
		  DMD_TIMER_SECOND_TIMEOUT_B = u16DMD_DVBC_AutoSymbol_Timeout;
	  }
	  else
	  {
		  DMD_TIMER_SECOND_TIMEOUT_B = (eQamMode == DMD_DVBC_QAMAUTO) ? u16DMD_DVBC_FixSymbol_AutoQam_Timeout : u16DMD_DVBC_FixSymbol_FixQam_Timeout;
	  }
  }  	



  //==========================================================
  //START: Create timer ID
  
  
  
  if (previous_demod_index==0x00)
  {
   
	  if((_s32DmdTimerId_FIRST_TIMEOUT_A >= 0) || (_s32DmdTimerId_SECOND_TIMEOUT_A >=0))  
	  {
		  MsOS_DeleteTimer(_s32DmdTimerId_FIRST_TIMEOUT_A);
		  MsOS_DeleteTimer(_s32DmdTimerId_SECOND_TIMEOUT_A);
		  _s32DmdTimerId_FIRST_TIMEOUT_A=-1;
		  _s32DmdTimerId_SECOND_TIMEOUT_A=-1;
	  DMD_DBG(printf("===> [104] delete the timer ID A \n"));   
	  }
  
	  if((_s32DmdTimerId_FIRST_TIMEOUT_A < 0) && (_s32DmdTimerId_SECOND_TIMEOUT_A <0))   
	  { 	 
		  _s32DmdTimerId_FIRST_TIMEOUT_A = MsOS_CreateTimer (MDrv_DMD_DVBC_Timer_CB,
											  DMD_TIMER_FIRST_TIMEOUT_A,
											  0,
											  FALSE,
											  "DMD_INTERN_DVBC_TIMER_FIRST");  


  
		  _s32DmdTimerId_SECOND_TIMEOUT_A = MsOS_CreateTimer (MDrv_DMD_DVBC_Timer_CB,
											  DMD_TIMER_SECOND_TIMEOUT_A,
											  0,
											  FALSE,
											  "DMD_INTERN_DVBC_TIMER_SECOND");	



	  DMD_DBG(printf("===> [106] create timer ID A \n"));
	  DMD_DBG(printf("===>	[109] DMD_TIMER_SECOND_TIMEOUT_A: %d ms\n",DMD_TIMER_SECOND_TIMEOUT_A));
		  
	    #ifdef MS_DEBUG
		  if (_s32DmdTimerId_FIRST_TIMEOUT_A > 0)			 
			  printf("[%s][%d] Timer 1 create ok\n",__FUNCTION__,__LINE__); 	  
		  else			  
			  printf("[%s][%d] create timer 1 failed \n",__FUNCTION__,__LINE__);   


		  if (_s32DmdTimerId_SECOND_TIMEOUT_A > 0)			  
			  printf("[%s][%d] Timer 2 create ok\n",__FUNCTION__,__LINE__); 	  
		  else			  
			  printf("[%s][%d] create timer 2 failed \n",__FUNCTION__,__LINE__); 
	    #endif   
	  }
  }
  else if (previous_demod_index==0x01)
  {
  
	  if((_s32DmdTimerId_FIRST_TIMEOUT_B >= 0) || (_s32DmdTimerId_SECOND_TIMEOUT_B >=0))  
	  {
		  MsOS_DeleteTimer(_s32DmdTimerId_FIRST_TIMEOUT_B);
		  MsOS_DeleteTimer(_s32DmdTimerId_SECOND_TIMEOUT_B);
		  _s32DmdTimerId_FIRST_TIMEOUT_B=-1;
		  _s32DmdTimerId_SECOND_TIMEOUT_B=-1;
	  DMD_DBG(printf("===> [104] delete the timer ID B \n"));   
	  } 
  
	  if((_s32DmdTimerId_FIRST_TIMEOUT_B < 0) && (_s32DmdTimerId_SECOND_TIMEOUT_B <0))   
	  { 	 
		  _s32DmdTimerId_FIRST_TIMEOUT_B = MsOS_CreateTimer (MDrv_DMD_DVBC_Timer_CB,
											  DMD_TIMER_FIRST_TIMEOUT_B,
											  0,
											  FALSE,
											  "DMD_INTERN_DVBC_TIMER_FIRST");  


  
		  _s32DmdTimerId_SECOND_TIMEOUT_B = MsOS_CreateTimer (MDrv_DMD_DVBC_Timer_CB,
											  DMD_TIMER_SECOND_TIMEOUT_B,
											  0,
											  FALSE,
											  "DMD_INTERN_DVBC_TIMER_SECOND");	



	  DMD_DBG(printf("===> [106] create timer ID B \n"));
	  DMD_DBG(printf("===>	[109] DMD_TIMER_SECOND_TIMEOUT_B: %d ms\n",DMD_TIMER_SECOND_TIMEOUT_B));

	    #ifdef MS_DEBUG
		  if (_s32DmdTimerId_FIRST_TIMEOUT_B > 0)			 
			  printf("[%s][%d] Timer 1 create ok\n",__FUNCTION__,__LINE__); 	  
		  else			  
			  printf("[%s][%d] create timer 1 failed \n",__FUNCTION__,__LINE__);   


		  if (_s32DmdTimerId_SECOND_TIMEOUT_B > 0)			  
			  printf("[%s][%d] Timer 2 create ok\n",__FUNCTION__,__LINE__); 	  
		  else			  
			  printf("[%s][%d] create timer 2 failed \n",__FUNCTION__,__LINE__); 
	    #endif   
	  }

  
  }
  //END: Create timer ID
  //==========================================================
 #endif 
 
  if(u32DVBCopen==1)
  {
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_SetSetConfig_symbol_rate_list,&Drv_DVBC_SetConfig_Symbol_rate_list_PARAM);
  }
  else
  {
    return false;
  }

  return Drv_DVBC_SetConfig_Symbol_rate_list_PARAM.ret;
}

MS_BOOL MDrv_DMD_DVBC_SetActive(MS_BOOL bEnable)
{
  DVBC_SetActive_PARAM Drv_DVBC_SetActive_PARAM;
  Drv_DVBC_SetActive_PARAM.bEnable=bEnable;
  Drv_DVBC_SetActive_PARAM.ret=false;
    DMD_DBG(ULOGD("DEMOD","[drvDMD_INTERN_DVBC.c]MDrv_DMD_DVBC_SetActive\n"));

  #ifdef MS_DEBUG
  ULOGD("DEMOD","check driver layer DVBC SetActive!!\n");
  #endif


#ifdef UFO_DEMOD_DVBC_SUPPORT_DMD_INT
	MsOS_EnableInterrupt(E_INT_FIQ_DMDMCU2HK); 
	if(bEnable==true)
	{

	//START: Start the timer
	 if (previous_demod_index==0x00)
		{
	    if (_s32DmdTimerId_FIRST_TIMEOUT_A > 0)    
		{        
			MsOS_StartTimer(_s32DmdTimerId_FIRST_TIMEOUT_A);
			DMD_DBG(printf("===>  [009] Start the timer: _s32DmdTimerId_FIRST_TIMEOUT_A   \n"));
		}
	    if (_s32DmdTimerId_SECOND_TIMEOUT_A > 0)    
		{        
			MsOS_StartTimer(_s32DmdTimerId_SECOND_TIMEOUT_A);
			DMD_DBG(printf("===>  [010] Start the timer: _s32DmdTimerId_SECOND_TIMEOUT_A   \n"));
		}	
	}
	else if (previous_demod_index==0x01)
	{
	    if (_s32DmdTimerId_FIRST_TIMEOUT_B > 0)    
		{        
			MsOS_StartTimer(_s32DmdTimerId_FIRST_TIMEOUT_B);
			DMD_DBG(printf("===>  [011] Start the timer: _s32DmdTimerId_FIRST_TIMEOUT_B   \n"));
		}
	    if (_s32DmdTimerId_SECOND_TIMEOUT_B > 0)    
		{        
			MsOS_StartTimer(_s32DmdTimerId_SECOND_TIMEOUT_B);
			DMD_DBG(printf("===>  [012] Start the timer: _s32DmdTimerId_SECOND_TIMEOUT_B   \n"));
           }
        }
    }
    else
    {
         printf("## setActive = FALSE\n");

        //START: Start the timer
         if (previous_demod_index==0x00)
         {
            if (_s32DmdTimerId_FIRST_TIMEOUT_A > 0)
            {
                MsOS_StopTimer(_s32DmdTimerId_FIRST_TIMEOUT_A);
                DMD_DBG(printf("===>  [009] Start the timer: _s32DmdTimerId_FIRST_TIMEOUT_A   \n"));
            }
            if (_s32DmdTimerId_SECOND_TIMEOUT_A > 0)
            {
                MsOS_StopTimer(_s32DmdTimerId_SECOND_TIMEOUT_A);
                DMD_DBG(printf("===>  [010] Start the timer: _s32DmdTimerId_SECOND_TIMEOUT_A   \n"));
            }
         }
        else if (previous_demod_index==0x01)
        {
            if (_s32DmdTimerId_FIRST_TIMEOUT_B > 0)
            {
                MsOS_StopTimer(_s32DmdTimerId_FIRST_TIMEOUT_B);
                DMD_DBG(printf("===>  [011] Start the timer: _s32DmdTimerId_FIRST_TIMEOUT_B   \n"));
            }
            if (_s32DmdTimerId_SECOND_TIMEOUT_B > 0)
            {
                MsOS_StopTimer(_s32DmdTimerId_SECOND_TIMEOUT_B);
                DMD_DBG(printf("===>  [012] Start the timer: _s32DmdTimerId_SECOND_TIMEOUT_B   \n"));
            }
        }
	//END: reset the timer

	}
#endif


  if(u32DVBCopen==1)
  {
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_SetActive,&Drv_DVBC_SetActive_PARAM);
  }
  else
  {
    return false;
  }

  
  return Drv_DVBC_SetActive_PARAM.ret;
}


    #ifdef UFO_DEMOD_DVBC_SUPPORT_DMD_INT

MS_BOOL MDrv_DMD_DVBC_Get_Info_Generalization(DMD_DVBC_GET_INFO_FUNC InfoType, void *pin, void *pout)
{
	DVBC_Get_Info_Generalization_PARAM  PARAM;

	DMD_DBG(printf("===> [131]  MDrv_DMD_DVBC_Get_Info_Generalization  \n"));
	
	if(u32DVBCopen==1)
	{
		switch(InfoType)
		{
			case GET_INFO_TR_EVER_LOCK:

				PARAM.InfoType=InfoType;
				PARAM.pin=(DVBC_GET_INFO_TR_EVER_LOCK_IN *)(PARAM.pin);
				PARAM.pout=(DVBC_GET_INFO_TR_EVER_LOCK_OUT *)(PARAM.pout);

				PARAM.pin=(DVBC_GET_INFO_TR_EVER_LOCK_IN *)pin;
				PARAM.pout=(DVBC_GET_INFO_TR_EVER_LOCK_OUT *)pout;
				PARAM.ret=((DVBC_GET_INFO_TR_EVER_LOCK_OUT *)pout)->ret;

					
   				UtopiaIoctl(ppDVBCInstant, DMD_DVBC_DRV_CMD_Get_Info_Generalization, &PARAM);

				return PARAM.ret;
				
			break;
			default:
				return false;
			break;
}

}
	
	else
	{
		return false; 
	}
}

#endif

#ifndef MSOS_TYPE_LINUX_KERNEL
MS_BOOL MDrv_DMD_DVBC_GetLock(DMD_DVBC_GETLOCK_TYPE eType, DMD_DVBC_LOCK_STATUS *eLockStatus)
{
    return MDrv_DMD_DVBC_GetLockWithRFPower(eType, eLockStatus, 200.0f, -200.0f);
}
#endif

#ifndef MSOS_TYPE_LINUX_KERNEL
MS_BOOL MDrv_DMD_DVBC_GetLockWithRFPower(DMD_DVBC_GETLOCK_TYPE eType, DMD_DVBC_LOCK_STATUS *eLockStatus, float fCurrRFPowerDbm, float fNoChannelRFPowerDbm)
{
  MS_BOOL status = true;
  
  DMD_IFAGC_SSI   *ifagc_ssi;
  DMD_IFAGC_ERR   *ifagc_err;
  float   ch_power_rf=0.0f;
  float   ch_power_db=0.0f;
  float   ch_power_if=0.0f, ch_power_ifa = 0.0f, ch_power_ifb =0.0f;
  MS_U16  if_agc_val =0, if_agc_vala =0, if_agc_valb =0, if_agc_val_lsb =0, i;
  float   ch_power_takeover=0.0f;
  MS_U8   ssi_tbl_len = 0, err_tbl_len = 0;
  
  MS_U8 ifagc_reg;
  MS_U8 ifagc_reg_lsb;
  MS_U16 ifagc_err_reg;

  DVBC_GetLockWithRFPower Drv_DVBC_GetLockWithRFPower;

  Drv_DVBC_GetLockWithRFPower.eType=eType;
  Drv_DVBC_GetLockWithRFPower.eLockStatus=eLockStatus;
  //Drv_DVBC_GetLockWithRFPower.u32CurrRFPowerDbm=(MS_U32)(fCurrRFPowerDbm*10);
  //Drv_DVBC_GetLockWithRFPower.u32NoChannelRFPowerDbm=(MS_U32)(fNoChannelRFPowerDbm*10);
  Drv_DVBC_GetLockWithRFPower.u32TimeInterval=0;
  Drv_DVBC_GetLockWithRFPower.ret=false;
  
  DVBC_GetIFAGC_PARAM Drv_DVBC_GetIFAGC_PARAM;
  Drv_DVBC_GetIFAGC_PARAM.ifagc_reg=&ifagc_reg;
  Drv_DVBC_GetIFAGC_PARAM.ifagc_reg_lsb=&ifagc_reg_lsb;
  Drv_DVBC_GetIFAGC_PARAM.ifagc_err_reg=&ifagc_err_reg;
  Drv_DVBC_GetIFAGC_PARAM.ret=false;

  if(u32DVBCopen==1)
  {
  	if((AgcSsi_Para.pTuner_IfagcSsi_HiRef != NULL) && (AgcSsi_Para.pTuner_IfagcSsi_LoRef != NULL))
    {
  	    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetIFAGC,&Drv_DVBC_GetIFAGC_PARAM);
        status &= Drv_DVBC_GetIFAGC_PARAM.ret;
    
        ch_power_rf = fCurrRFPowerDbm;
        if_agc_val = ifagc_reg;
        if_agc_val_lsb = ifagc_reg_lsb;
    
        ifagc_ssi = AgcSsi_Para.pTuner_IfagcSsi_LoRef;
        ssi_tbl_len = AgcSsi_Para.u16Tuner_IfagcSsi_LoRef_Size;
        ifagc_err = AgcSsi_Para.pTuner_IfagcErr_LoRef;
        err_tbl_len = AgcSsi_Para.u16Tuner_IfagcErr_LoRef_Size;   
    
        ch_power_if=ifagc_ssi[0].power_db;
        if (if_agc_val >=ifagc_ssi[0].agc_val)
        {
                for(i = 1; i < ssi_tbl_len; i++)
                {
                    if (if_agc_val < ifagc_ssi[i].agc_val)
                    {
                        if_agc_valb = ifagc_ssi[i].agc_val;
                        ch_power_ifb = ifagc_ssi[i].power_db;
        
                        i--;
                        if_agc_vala = ifagc_ssi[i].agc_val;
                        ch_power_ifa=ifagc_ssi[i].power_db;
                        while ((i>1) && (if_agc_vala==ifagc_ssi[i-1].agc_val))
                        {
                            ch_power_ifa=ifagc_ssi[i-1].power_db;
                            i--;
                        }
                        ch_power_if = ch_power_ifa+(ch_power_ifb-ch_power_ifa)*(float)((if_agc_val-if_agc_vala)*256+if_agc_val_lsb)/((if_agc_valb-if_agc_vala)*256);
                        break;
                    }
                }
        }
            #ifdef MS_DEBUG
            ULOGD("DEMOD","if prev %f %x\n", ch_power_ifa, if_agc_vala);
            ULOGD("DEMOD","if next %f %x\n", ch_power_ifb, if_agc_valb);  
            #endif

            
            for(i = 0; i < ssi_tbl_len; i++)
            {
                if (ifagc_ssi[i].agc_val <= ifagc_ssi[i+1].agc_val)
                {
                    ch_power_takeover = ifagc_ssi[i+1].power_db;
                    break;
                }
            }

            #ifdef MS_DEBUG
            //ULOGD("DEMOD","ch_power_rf = %f\n", ch_power_rf);
            ULOGD("DEMOD","ch_power_if = %f\n", ch_power_if);
            ULOGD("DEMOD","ch_power_takeover = %f\n", ch_power_takeover);
            #endif

            // ch_power_db = (ch_power_rf > ch_power_if)? ch_power_rf : ch_power_if;

            if(ch_power_rf > (ch_power_takeover + 0.5))
            {
                ch_power_db = ch_power_rf;
            }
            else if(ch_power_if < (ch_power_takeover - 0.5))
            {
                ch_power_db = ch_power_if;
            }
            else
            {
                ch_power_db = (ch_power_if + ch_power_rf)/2;
            }
          
            // ch_power_db = (ch_power_rf > ch_power_if)? ch_power_if : ch_power_rf;
    
        if(if_agc_val == 0xff)
        {
            for(i = 0; i < err_tbl_len; i++)
            {
                    if ( ifagc_err_reg <= ifagc_err[i].agc_err )        // signed char comparison
                    {
                        ch_power_db += ifagc_err[i].attn_db;
                        break;
                    }
            }
            #ifdef MS_DEBUG
            ULOGD("DEMOD","if_agc_err = 0x%x\n", ifagc_err_reg);
            #endif
        }
    }
    else
    {
    	  #ifdef MS_DEBUG
        if (fCurrRFPowerDbm>=100.0) // unreasonable input value, get RF level from RFAGG
        {
            ULOGD("DEMOD","Error!! please add AGC table\n");
        }
        #endif
        ch_power_db = fCurrRFPowerDbm;
    }
    
  	Drv_DVBC_GetLockWithRFPower.u32CurrRFPowerDbm=(MS_U32)(ch_power_db*10);
    Drv_DVBC_GetLockWithRFPower.u32NoChannelRFPowerDbm=(MS_U32)(fNoChannelRFPowerDbm*10);
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetLockWithRFPower,&Drv_DVBC_GetLockWithRFPower);
    status &= Drv_DVBC_GetLockWithRFPower.ret;
  }
  else
  {
    return false;
  }
     
  #ifdef MS_DEBUG 
  //if(*(Drv_DVBC_GetLockWithRFPower_Transform.eLockStatus)==DMD_DVBC_LOCK)
  if(*(Drv_DVBC_GetLockWithRFPower.eLockStatus)==DMD_DVBC_LOCK)
  {
    ULOGD("DEMOD","check in drv layer DVBC demod locked!!\n");
  }
  else
  {
    ULOGD("DEMOD","check in drv layer DVBC demod unlock!!\n");
    return false;
  }
  #endif

  return status;
}
#endif

#ifndef MSOS_TYPE_LINUX_KERNEL
MS_BOOL MDrv_DMD_DVBC_GetSignalStrength(MS_U16 *u16Strength)
{
    return MDrv_DMD_DVBC_GetSignalStrengthWithRFPower(u16Strength, 200.0f);
}
#endif

#ifndef MSOS_TYPE_LINUX_KERNEL
MS_BOOL MDrv_DMD_DVBC_GetSignalStrengthWithRFPower(MS_U16 *u16Strength, float fRFPowerDbm)
{ 
  MS_BOOL status = true;
  DMD_IFAGC_SSI   *ifagc_ssi;
  DMD_IFAGC_ERR   *ifagc_err;
  float   ch_power_rf=0.0f;
  float   ch_power_db=0.0f, ch_power_db_rel=0.0f;
  float   ch_power_if=0.0f, ch_power_ifa = 0.0f, ch_power_ifb =0.0f;
  MS_U16  if_agc_val =0, if_agc_vala =0, if_agc_valb =0, if_agc_val_lsb =0, i;
  float   ch_power_takeover=0.0f;
  MS_U8   ssi_tbl_len = 0, err_tbl_len = 0;
  
  MS_U8 ifagc_reg;
  MS_U8 ifagc_reg_lsb;
  MS_U16 ifagc_err_reg;
  DMD_DVBC_MODULATION_TYPE Qam_mode = DMD_DVBC_QAM16;
  MS_U16 SymbolRate;
  float FreqOff;
  
  //DVBC_GetSignalStrengthWithRFPower_PARAM Drv_DVBC_GetSignalStrengthWithRFPower_PARAM;
  //Drv_DVBC_GetSignalStrengthWithRFPower_PARAM.u16Strength=u16Strength;
  //Drv_DVBC_GetSignalStrengthWithRFPower_PARAM.fRFPowerDbm=fRFPowerDbm;
  
  DVBC_GetIFAGC_PARAM Drv_DVBC_GetIFAGC_PARAM;
  Drv_DVBC_GetIFAGC_PARAM.ifagc_reg=&ifagc_reg;
  Drv_DVBC_GetIFAGC_PARAM.ifagc_reg_lsb=&ifagc_reg_lsb;
  Drv_DVBC_GetIFAGC_PARAM.ifagc_err_reg=&ifagc_err_reg;
  Drv_DVBC_GetIFAGC_PARAM.ret=false;

  if(u32DVBCopen==1)
  {
    if((AgcSsi_Para.pTuner_IfagcSsi_HiRef != NULL) && (AgcSsi_Para.pTuner_IfagcSsi_LoRef != NULL))
    {
    	  UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetIFAGC,&Drv_DVBC_GetIFAGC_PARAM);
        status &= Drv_DVBC_GetIFAGC_PARAM.ret;
        ch_power_rf = fRFPowerDbm;
    
        if_agc_val = ifagc_reg;
        if_agc_val_lsb = ifagc_reg_lsb;
    
        ifagc_ssi = AgcSsi_Para.pTuner_IfagcSsi_LoRef;
        ssi_tbl_len = AgcSsi_Para.u16Tuner_IfagcSsi_LoRef_Size;
        ifagc_err = AgcSsi_Para.pTuner_IfagcErr_LoRef;
        err_tbl_len = AgcSsi_Para.u16Tuner_IfagcErr_LoRef_Size;

        ch_power_if=ifagc_ssi[0].power_db;
        if (if_agc_val >=ifagc_ssi[0].agc_val)
        {
                for(i = 1; i < ssi_tbl_len; i++)
                {
                    if (if_agc_val < ifagc_ssi[i].agc_val)
                    {
                        if_agc_valb = ifagc_ssi[i].agc_val;
                        ch_power_ifb = ifagc_ssi[i].power_db;
        
                        i--;
                        if_agc_vala = ifagc_ssi[i].agc_val;
                        ch_power_ifa=ifagc_ssi[i].power_db;
                        while ((i>1) && (if_agc_vala==ifagc_ssi[i-1].agc_val))
                        {
                            ch_power_ifa=ifagc_ssi[i-1].power_db;
                            i--;
                        }
                        ch_power_if = ch_power_ifa+(ch_power_ifb-ch_power_ifa)*(float)((if_agc_val-if_agc_vala)*256+if_agc_val_lsb)/((if_agc_valb-if_agc_vala)*256);
                        break;
                    }
                }
        }
            #ifdef MS_DEBUG
            ULOGD("DEMOD","if prev %f %x\n", ch_power_ifa, if_agc_vala);
            ULOGD("DEMOD","if next %f %x\n", ch_power_ifb, if_agc_valb);  
            #endif
            
            for(i = 0; i < ssi_tbl_len; i++)
            {
                if (ifagc_ssi[i].agc_val <= ifagc_ssi[i+1].agc_val)
                {
                    ch_power_takeover = ifagc_ssi[i+1].power_db;
                    break;
                }
            }

            #ifdef MS_DEBUG
            ULOGD("DEMOD","ch_power_rf = %f\n", ch_power_rf);
            ULOGD("DEMOD","ch_power_if = %f\n", ch_power_if);
            ULOGD("DEMOD","ch_power_takeover = %f\n", ch_power_takeover);
            #endif

            // ch_power_db = (ch_power_rf > ch_power_if)? ch_power_rf : ch_power_if;

            if(ch_power_rf > (ch_power_takeover + 0.5))
            {
                ch_power_db = ch_power_rf;
            }
            else if(ch_power_if < (ch_power_takeover - 0.5))
            {
                ch_power_db = ch_power_if;
            }
            else
            {
                ch_power_db = (ch_power_if + ch_power_rf)/2;
            }
          
            // ch_power_db = (ch_power_rf > ch_power_if)? ch_power_if : ch_power_rf;
    
        if(if_agc_val == 0xff)
        {
            for(i = 0; i < err_tbl_len; i++)
            {
                    if ( ifagc_err_reg <= ifagc_err[i].agc_err )        // signed char comparison
                    {
                        ch_power_db += ifagc_err[i].attn_db;
                        break;
                    }
            }
            #ifdef MS_DEBUG
            ULOGD("DEMOD","if_agc_err = 0x%x\n", ifagc_err_reg);
           #endif
        }
    }
    else
    {
    	  #ifdef MS_DEBUG
        if (fRFPowerDbm>=100.0) // unreasonable input value, get RF level from RFAGG
        {
            ULOGD("DEMOD","Error!! please add AGC table\n");
        }
        #endif
        ch_power_db = fRFPowerDbm;
    }
                                                                
    status &= MDrv_DMD_DVBC_GetStatus(&Qam_mode, &SymbolRate, &FreqOff);
    
    if( (MS_U8)Qam_mode <= (MS_U8)DMD_DVBC_QAM256)
    {
        ch_power_db_rel = ch_power_db + intern_dvb_c_qam_ref[(MS_U8)Qam_mode];
    }
    else
    {
        ch_power_db_rel = -100.0f;
    }

    if(ch_power_db_rel <= -85.0f)
        {*u16Strength = 0;}
    else if (ch_power_db_rel <= -80.0f)
        {*u16Strength = (MS_U16)(0.0f + (ch_power_db_rel+85.0f)*10.0f/5.0f);}
    else if (ch_power_db_rel <= -75.0f)
        {*u16Strength = (MS_U16)(10.0f + (ch_power_db_rel+80.0f)*20.0f/5.0f);}
    else if (ch_power_db_rel <= -70.0f)
        {*u16Strength = (MS_U16)(30.0f + (ch_power_db_rel+75.0f)*30.0f/5.0f);}
    else if (ch_power_db_rel <= -65.0f)
        {*u16Strength = (MS_U16)(60.0f + (ch_power_db_rel+70.0f)*10.0f/5.0f);}
    else if (ch_power_db_rel <= -55.0f)
        {*u16Strength = (MS_U16)(70.0f + (ch_power_db_rel+65.0f)*20.0f/10.0f);}
    else if (ch_power_db_rel <= -45.0f)
        {*u16Strength = (MS_U16)(90.0f + (ch_power_db_rel+55.0f)*10.0f/10.0f);}
    else
        {*u16Strength = 100;}

    DMD_DBG(ULOGD("DEMOD",">>> SSI_CH_PWR(dB) = %f , Score = %d<<<\n", ch_power_db, *u16Strength));
    DMD_DBG(ULOGD("DEMOD",">>> SSI = %d <<<\n", (int)*u16Strength));

    return status;
  }
  else
  {
    return false;
  }

  return status;
}
#endif

#ifndef MSOS_TYPE_LINUX_KERNEL
MS_BOOL MDrv_DMD_DVBC_GetSignalQuality(MS_U16 *u16Quality)
{
    return MDrv_DMD_DVBC_GetSignalQualityWithRFPower(u16Quality, 200.0f);
}
#endif

#ifndef MSOS_TYPE_LINUX_KERNEL
MS_BOOL MDrv_DMD_DVBC_GetSignalQualityWithRFPower(MS_U16 *u16Quality, float fRFPowerDbm)
{
  float       fber;
  float       log_ber;
  MS_BOOL status = true;
  DMD_DVBC_MODULATION_TYPE Qam_mode = DMD_DVBC_QAM16;
  float f_snr = 0.0;
  
  DMD_DVBC_LOCK_STATUS eLockStatus = DMD_DVBC_CHECKING;
  MS_U16 SymbolRate;
  float FreqOff;
  

  if(u32DVBCopen==1)
  {
    status &= MDrv_DMD_DVBC_GetSNR(&f_snr);
    status &= MDrv_DMD_DVBC_GetLock(DMD_DVBC_GETLOCK_PSYNC_LOCK, &eLockStatus);
    if(eLockStatus == DMD_DVBC_LOCK)
    {
    	  if (MDrv_DMD_DVBC_GetPostViterbiBer(&fber) == FALSE)
        {
            DMD_DBG(ULOGD("DEMOD","\nGetPostViterbiBer Fail!"));
            return FALSE;
        }
        
        // log_ber = log10(fber)
        #ifdef MSOS_TYPE_LINUX
          log_ber = (-1.0f)*log10f(1.0f/fber);
        #else
          log_ber = (-1.0f)*Log10Approx(1.0f/fber); // Log10Approx() provide 1~2^32 input range only
        #endif

        DMD_DBG(ULOGD("DEMOD","\nLog(BER) = %f",log_ber));
        status &= MDrv_DMD_DVBC_GetStatus(&Qam_mode, &SymbolRate, &FreqOff);    
        if (Qam_mode == DMD_DVBC_QAM16)
        {
            if(log_ber  <= (-5.5f))
                *u16Quality = 100;
            else if(log_ber  <= (-5.1f))
                *u16Quality = (MS_U16)(90.0f + ((-5.1f)-log_ber)*10.0f/((-5.1f)-(-5.5f)));
            else if(log_ber  <= (-4.9f))
                *u16Quality = (MS_U16)(80.0f + ((-4.9f)-log_ber)*10.0f/((-4.9f)-(-5.1f)));
            else if(log_ber  <= (-4.5f))
                *u16Quality = (MS_U16)(70.0f + ((-4.5f)-log_ber)*10.0f/((-4.5f)-(-4.9f)));
            else if(log_ber  <= (-3.7f))
                *u16Quality = (MS_U16)(60.0f + ((-3.7f)-log_ber)*10.0f/((-3.7f)-(-4.5f)));
            else if(log_ber  <= (-3.2f))
                *u16Quality = (MS_U16)(50.0f + ((-3.2f)-log_ber)*10.0f/((-3.2f)-(-3.7f)));
            else if(log_ber  <= (-2.9f))
                *u16Quality = (MS_U16)(40.0f + ((-2.9f)-log_ber)*10.0f/((-2.9f)-(-3.2f)));
            else if(log_ber  <= (-2.5f))
                *u16Quality = (MS_U16)(30.0f + ((-2.5f)-log_ber)*10.0f/((-2.5f)-(-2.9f)));
            else if(log_ber  <= (-2.2f))
                *u16Quality = (MS_U16)(20.0f + ((-2.2f)-log_ber)*10.0f/((-2.2f)-(-2.5f)));
            else if(log_ber  <= (-2.0f))
                *u16Quality = (MS_U16)(0.0f + ((-2.0f)-log_ber)*10.0f/((-2.0f)-(-2.2f)));
            else
                *u16Quality = 0;
        }
        else if (Qam_mode == DMD_DVBC_QAM32)
        {
            if(log_ber  <= (-5.0f))
                *u16Quality = 100;
            else if(log_ber  <= (-4.7f))
                *u16Quality = (MS_U16)(90.0f  + ((-4.7f)-log_ber)*10.0f/((-4.7f)-(-5.0f)));
            else if(log_ber  <= (-4.5f))
                *u16Quality = (MS_U16)(80.0f  + ((-4.5f)-log_ber)*10.0f/((-4.5f)-(-4.7f)));
            else if(log_ber  <= (-3.8f))
                *u16Quality = (MS_U16)(70.0f  + ((-3.8f)-log_ber)*10.0f/((-3.8f)-(-4.5f)));
            else if(log_ber  <= (-3.5f))
                *u16Quality = (MS_U16)(60.0f  + ((-3.5f)-log_ber)*10.0f/((-3.5f)-(-3.8f)));
            else if(log_ber  <= (-3.0f))
                *u16Quality = (MS_U16)(50.0f  + ((-3.0f)-log_ber)*10.0f/((-3.0f)-(-3.5f)));
            else if(log_ber  <= (-2.7f))
                *u16Quality = (MS_U16)(40.0f  + ((-2.7f)-log_ber)*10.0f/((-2.7f)-(-3.0f)));
            else if(log_ber  <= (-2.4f))
                *u16Quality = (MS_U16)(30.0f  + ((-2.4f)-log_ber)*10.0f/((-2.4f)-(-2.7f)));
            else if(log_ber  <= (-2.2f))
                *u16Quality = (MS_U16)(20.0f  + ((-2.2f)-log_ber)*10.0f/((-2.2f)-(-2.4f)));
            else if(log_ber  <= (-2.0f))
                *u16Quality = (MS_U16)(0.0f  + ((-2.0f)-log_ber)*10.0f/((-2.0f)-(-2.2f)));
            else
                *u16Quality = 0;
        }
        else if (Qam_mode == DMD_DVBC_QAM64)
        {
            if(log_ber  <= (-5.4f))
                *u16Quality = 100;
            else if(log_ber  <= (-5.1f))
                *u16Quality = (MS_U16)(90.0f + ((-5.1f)-log_ber)*10.0f/((-5.1f)-(-5.4f)));
            else if(log_ber  <= (-4.9f))
                *u16Quality = (MS_U16)(80.0f + ((-4.9f)-log_ber)*10.0f/((-4.9f)-(-5.1f)));
            else if(log_ber  <= (-4.3f))
                *u16Quality = (MS_U16)(70.0f + ((-4.3f)-log_ber)*10.0f/((-4.3f)-(-4.9f)));
            else if(log_ber  <= (-3.7f))
                *u16Quality = (MS_U16)(60.0f + ((-3.7f)-log_ber)*10.0f/((-3.7f)-(-4.3f)));
            else if(log_ber  <= (-3.2f))
                *u16Quality = (MS_U16)(50.0f + ((-3.2f)-log_ber)*10.0f/((-3.2f)-(-3.7f)));
            else if(log_ber  <= (-2.9f))
                *u16Quality = (MS_U16)(40.0f + ((-2.9f)-log_ber)*10.0f/((-2.9f)-(-3.2f)));
            else if(log_ber  <= (-2.4f))
                *u16Quality = (MS_U16)(30.0f + ((-2.4f)-log_ber)*10.0f/((-2.4f)-(-2.9f)));
            else if(log_ber  <= (-2.2f))
                *u16Quality = (MS_U16)(20.0f + ((-2.2f)-log_ber)*10.0f/((-2.2f)-(-2.4f)));
            else if(log_ber  <= (-2.05f))
                *u16Quality = (MS_U16)(0.0f + ((-2.05f)-log_ber)*10.0f/((-2.05f)-(-2.2f)));
            else
                *u16Quality = 0;
        }
        else if (Qam_mode == DMD_DVBC_QAM128)
        {
            if(log_ber  <= (-5.1f))
            *u16Quality = 100;
            else if(log_ber  <= (-4.9f))
            *u16Quality = (MS_U16)(90.0f + ((-4.9f)-log_ber)*10.0f/((-4.9f)-(-5.1f)));
            else if(log_ber  <= (-4.7f))
            *u16Quality = (MS_U16)(80.0f + ((-4.7f)-log_ber)*10.0f/((-4.7f)-(-4.9f)));
            else if(log_ber  <= (-4.1f))
            *u16Quality = (MS_U16)(70.0f + ((-4.1f)-log_ber)*10.0f/((-4.1f)-(-4.7f)));
            else if(log_ber  <= (-3.5f))
            *u16Quality = (MS_U16)(60.0f + ((-3.5f)-log_ber)*10.0f/((-3.5f)-(-4.1f)));
            else if(log_ber  <= (-3.1f))
            *u16Quality = (MS_U16)(50.0f + ((-3.1f)-log_ber)*10.0f/((-3.1f)-(-3.5f)));
            else if(log_ber  <= (-2.7f))
            *u16Quality = (MS_U16)(40.0f + ((-2.7f)-log_ber)*10.0f/((-2.7f)-(-3.1f)));
            else if(log_ber  <= (-2.5f))
            *u16Quality = (MS_U16)(30.0f + ((-2.5f)-log_ber)*10.0f/((-2.5f)-(-2.7f)));
            else if(log_ber  <= (-2.06f))
            *u16Quality = (MS_U16)(20.0f + ((-2.06f)-log_ber)*10.0f/((-2.06f)-(-2.5f)));
        //else if(log_ber  <= (-2.05))
        else
        {
            if (f_snr >= 27.2f)
            *u16Quality = 20;
            else if (f_snr >= 25.1f)
            *u16Quality = (MS_U16)(0.0f + (f_snr - 25.1f)*20.0f/(27.2f-25.1f));
            else
            *u16Quality = 0;
        }
        }
        else //256QAM
        {
            if(log_ber  <= (-4.8f))
                *u16Quality = 100;
            else if(log_ber  <= (-4.6f))
                *u16Quality = (MS_U16)(90.0f + ((-4.6f)-log_ber)*10.0f/((-4.6f)-(-4.8f)));
            else if(log_ber  <= (-4.4f))
                *u16Quality = (MS_U16)(80.0f + ((-4.4f)-log_ber)*10.0f/((-4.4f)-(-4.6f)));
            else if(log_ber  <= (-4.0f))
                *u16Quality = (MS_U16)(70.0f + ((-4.0f)-log_ber)*10.0f/((-4.0f)-(-4.4f)));
            else if(log_ber  <= (-3.5f))
                *u16Quality = (MS_U16)(60.0f + ((-3.5f)-log_ber)*10.0f/((-3.5f)-(-4.0f)));
            else if(log_ber  <= (-3.1f))
                *u16Quality = (MS_U16)(50.0f + ((-3.1f)-log_ber)*10.0f/((-3.1f)-(-3.5f)));
            else if(log_ber  <= (-2.7f))
                *u16Quality = (MS_U16)(40.0f + ((-2.7f)-log_ber)*10.0f/((-2.7f)-(-3.1f)));
            else if(log_ber  <= (-2.4f))
                *u16Quality = (MS_U16)(30.0f + ((-2.4f)-log_ber)*10.0f/((-2.4f)-(-2.7f)));
            else if(log_ber  <= (-2.06f))
                *u16Quality = (MS_U16)(20.0f + ((-2.06f)-log_ber)*10.0f/((-2.06f)-(-2.4f)));
        //else if(log_ber  <= (-2.05))
        else
        {
            if (f_snr >= 29.6f)
                *u16Quality = 20;
            else if (f_snr >= 27.3f)
                *u16Quality = (MS_U16)(0.0f + (f_snr - 27.3f)*20.0f/(29.6f-27.3f));
            else
                *u16Quality = 0;
        }
        }
    }
    else
    {
        *u16Quality = 0;
    }
    
    DMD_DBG(ULOGD("DEMOD","BER = %8.3e\n", fber));
    DMD_DBG(ULOGD("DEMOD","Signal Quility = %d\n", *u16Quality));
    return TRUE;  
  }
  else
  {
    return false;
  }

  return status;  
}
#endif

#if defined(CHIP_KAISER)||defined(CHIP_K6LITE)
MS_BOOL MDrv_DMD_DVBC_ActiveDmdSwitch(MS_U8 demod_no)
{
  DVBC_ActiveDmdSwitch_PARAM Drv_DVBC_ActiveDmdSwitch_PARAM;
  Drv_DVBC_ActiveDmdSwitch_PARAM.demod_no=demod_no;

  if(u32DVBCopen==1)
  {
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_ActiveDmdSwitch,&Drv_DVBC_ActiveDmdSwitch_PARAM);

		if(demod_no==0 && previous_demod_index==1)
		{
				AgcSsi_Para_1=AgcSsi_Para;
				AgcSsi_Para=AgcSsi_Para_0;
				
		}
		else if(demod_no==1 && previous_demod_index==0)
		{
				AgcSsi_Para_0=AgcSsi_Para;
				AgcSsi_Para=AgcSsi_Para_1;
		}
		
		previous_demod_index=demod_no;
  }
  else
  {
    return false;
  }

  return Drv_DVBC_ActiveDmdSwitch_PARAM.ret;
 }
#else
MS_BOOL MDrv_DMD_DVBC_ActiveDmdSwitch(MS_U8 demod_no)
{
  ULOGD("DEMOD","Doesn't support DVBC_ActiveDmdSwitch function!!!\n");
  return false;
}
#endif

#ifndef MSOS_TYPE_LINUX_KERNEL
MS_BOOL MDrv_DMD_DVBC_GetSNR(float *fSNR)
{
	MS_U16 snr_reg;

  DVBC_GetSNR_PARAM Drv_DVBC_GetSNR_PARAM;
  Drv_DVBC_GetSNR_PARAM.snr_reg=&snr_reg;
  Drv_DVBC_GetSNR_PARAM.ret = false;

  if(u32DVBCopen==1)
  {
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetSNR,&Drv_DVBC_GetSNR_PARAM);

#ifdef MSOS_TYPE_LINUX
    *fSNR = 10.0f*log10f(65536.0f/(float)(snr_reg));
#else
    *fSNR = 10.0f*Log10Approx(65536.0f/(float)(snr_reg));
#endif

	Drv_DVBC_GetSNR_PARAM.ret = true;
  }
  else
  {
    return false;
  }   

  return Drv_DVBC_GetSNR_PARAM.ret;
}
#endif

#ifndef MSOS_TYPE_LINUX_KERNEL
MS_BOOL MDrv_DMD_DVBC_GetPostViterbiBer(float *ber)
{
  MS_U16 BitErrPeriod_reg;
  MS_U32 BitErr_reg;
  MS_U16 PktErr;
  
  DVBC_GetPostViterbiBer_PARAM Drv_DVBC_GetPostViterbiBer_PARAM;
  Drv_DVBC_GetPostViterbiBer_PARAM.BitErr_reg=&BitErr_reg;
  Drv_DVBC_GetPostViterbiBer_PARAM.BitErrPeriod_reg=&BitErrPeriod_reg;
  Drv_DVBC_GetPostViterbiBer_PARAM.ret=false; 
  

  if(u32DVBCopen==1)
  {
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetPostViterbiBer,&Drv_DVBC_GetPostViterbiBer_PARAM);
    MDrv_DMD_DVBC_GetPacketErr(&PktErr);
    
    if (BitErrPeriod_reg == 0 )    //protect 0
        BitErrPeriod_reg = 1;

    if (BitErr_reg <=0 )
    	{
        *ber = 0.5f / ((float)(BitErrPeriod_reg)*128*188*8);
    	}
    else
    	{
        *ber = (float)BitErr_reg / ((float)(BitErrPeriod_reg)*128*188*8);
    	}
    DMD_DBG(ULOGD("DEMOD","PostVitBER = %8.3e \n ", *ber));
    DMD_DBG(ULOGD("DEMOD","PktErr = %d \n ", (int)PktErr));
	Drv_DVBC_GetPostViterbiBer_PARAM.ret=true; 
  }
  else
  {
    return false;
  }   


  return Drv_DVBC_GetPostViterbiBer_PARAM.ret;
}
#endif

MS_BOOL MDrv_DMD_DVBC_GetPacketErr(MS_U16 *pktErr)
{
    DVBC_GetPacketErr_PARAM Drv_DVBC_GetPacketErr_PARAM;
    Drv_DVBC_GetPacketErr_PARAM.pktErr=pktErr;
  Drv_DVBC_GetPacketErr_PARAM.ret=false;

    if(u32DVBCopen==1)
  {
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetPacketErr,&Drv_DVBC_GetPacketErr_PARAM);
  }
  else
  {
    return false;
  }   

  return Drv_DVBC_GetPacketErr_PARAM.ret;
}

MS_BOOL MDrv_DMD_DVBC_GetCellID(MS_U16 *u16CellID)
{
  DVBC_GetCellID_PARAM Drv_DVBC_GetCellID_PARAM;
  Drv_DVBC_GetCellID_PARAM.u16CellID=u16CellID;
  Drv_DVBC_GetCellID_PARAM.ret=false;

  if(u32DVBCopen==1)
  {
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetCellID,&Drv_DVBC_GetCellID_PARAM);
  }
  else
  {
    return false;
  }   

  return Drv_DVBC_GetCellID_PARAM.ret;
  
}


#ifndef MSOS_TYPE_LINUX_KERNEL
MS_BOOL MDrv_DMD_DVBC_GetStatus(DMD_DVBC_MODULATION_TYPE *pQAMMode, MS_U16 *u16SymbolRate, float *pFreqOff)
{
	MS_U32 config_Fc_reg;
	MS_U32 Fc_over_Fs_reg;
	MS_U16 Cfo_offset_reg;

  DVBC_GetStatus_PARAM Drv_DVBC_GetStatus_PARAM;
  Drv_DVBC_GetStatus_PARAM.pQAMMode=pQAMMode;
  Drv_DVBC_GetStatus_PARAM.u16SymbolRate=u16SymbolRate;
  Drv_DVBC_GetStatus_PARAM.config_Fc_reg=&config_Fc_reg;
  Drv_DVBC_GetStatus_PARAM.Fc_over_Fs_reg=&Fc_over_Fs_reg;
  Drv_DVBC_GetStatus_PARAM.Cfo_offset_reg=&Cfo_offset_reg; // FC
  Drv_DVBC_GetStatus_PARAM.ret=false;
  
  float f_Fc;// FreqCfo_offset;
  
  if(u32DVBCopen==1)
  {
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetStatus,&Drv_DVBC_GetStatus_PARAM);

      f_Fc = (float)Fc_over_Fs_reg/134217728.0f * 48000.0f;
   /*   
    FreqCfo_offset = (MS_S32)(Cfo_offset_reg<<4)/16;

    FreqCfo_offset = FreqCfo_offset/0x8000000/8.0f;

    FreqCfo_offset = FreqCfo_offset * (*u16SymbolRate) - (f_Fc-(float)config_Fc_reg);

    *pFreqOff = FreqCfo_offset;
    */
    *pFreqOff = f_Fc - (48000-Cfo_offset_reg);

    #ifdef MS_DEBUG
    if (_u8DMD_DVBC_DbgLevel >= DMD_DVBC_DBGLV_INFO)
    {
        ULOGD("DEMOD","MDrv_DMD_DVBC_GetStatus %d %d %f\n", *pQAMMode, *u16SymbolRate, *pFreqOff);
    }
    #endif
  }
  else
  {
    return false;
  }   

  return Drv_DVBC_GetStatus_PARAM.ret; 
}
#endif

MS_U32 MDrv_DMD_DVBC_SetPowerState(EN_POWER_MODE u16PowerState)
{
    DVBC_SetPowerState_PARAM Drv_DVBC_SetPowerState_PARAM;
    Drv_DVBC_SetPowerState_PARAM.u16PowerState=u16PowerState;
    Drv_DVBC_SetPowerState_PARAM.ret_U32=false;
   DMD_DBG(ULOGD("DEMOD","[drvDMD_INTERN_DVBC.c]MDrv_DMD_DVBC_SetPowerState\n"));
   if(u32DVBCopen==1)
   {
  UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_SetPowerState,&Drv_DVBC_SetPowerState_PARAM);
   }
   else
   {
  return false;
    }   

    return Drv_DVBC_SetPowerState_PARAM.ret_U32;
}

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
MS_BOOL MDrv_DMD_DVBC_GetMDBConfig(MS_U16 *u16SymbolRate, MS_U8 *eQamMode, MS_U32 *u32IFFreq, MS_BOOL *bSpecInv, MS_BOOL *bSerialTS)
{
	  MS_U8 u8_reg, u8_reg1;
	  MS_U16 u16_reg;
	  MS_U8 status = true;

    //SR
    status &= MDrv_DMD_DVBC_GetDSPReg(E_DMD_DVBC_OP_AUTO_SCAN_SYM_RATE, &u8_reg1);
    if(u8_reg1 == 0x00){
    status &= MDrv_DMD_DVBC_GetDSPReg(E_DMD_DVBC_CFG_BW0_H, &u8_reg);
    u16_reg = u8_reg;
    status &= MDrv_DMD_DVBC_GetDSPReg(E_DMD_DVBC_CFG_BW0_L, &u8_reg);
    u16_reg = (u16_reg<<8)|u8_reg;
	  }
	  else
	  u16_reg = 0;
	  
	  *u16SymbolRate = u16_reg;

	  //QAM
	  status &= MDrv_DMD_DVBC_GetDSPReg(E_DMD_DVBC_OP_AUTO_SCAN_QAM, &u8_reg1);
    if(u8_reg1 == 0x00){
    status &= MDrv_DMD_DVBC_GetDSPReg(E_DMD_DVBC_CFG_QAM, &u8_reg);
    *eQamMode = u8_reg;
	  }
	  else
	  *eQamMode = DMD_DVBC_QAMAUTO; 

	  //IF
    status &= MDrv_DMD_DVBC_GetDSPReg(E_DMD_DVBC_CFG_FIF_H, &u8_reg);
    u16_reg = u8_reg;
    status &= MDrv_DMD_DVBC_GetDSPReg(E_DMD_DVBC_CFG_FIF_L, &u8_reg);
	  u16_reg = (u16_reg<<8)|u8_reg;
	  
	  *u32IFFreq = u16_reg;
	  
	  //bSpecInv, IQ Swap
	  status &= MDrv_DMD_DVBC_GetDSPReg(E_DMD_DVBC_CFG_IQ_SWAP, &u8_reg);
	  *bSpecInv = u8_reg;
 
	  //bSerialTS, TS mode
    status &= MDrv_DMD_DVBC_GetDSPReg(E_DMD_DVBC_CFG_TS_SERIAL, &u8_reg);
    *bSerialTS = u8_reg;
      
     if(status == 0)
     ULOGD("DEMOD","[drvDMD_INTERN_DVBC.c] MDrv_DMD_DVBC_GetMDBConfig Fail\n");
     
	  return status;
}

#ifdef MSOS_TYPE_LINUX_KERNEL
MS_BOOL MDrv_DMD_DVBC_GetMdbStatus(DMD_DVBC_MODULATION_TYPE *pQAMMode, MS_U16 *u16SymbolRate, MS_S32 *pFreqOff, DMD_DVBC_GETLOCK_TYPE eType, DMD_DVBC_LOCK_STATUS *eLockStatus,MS_U8 SNR)
{
	// Re-link info. For procfs implement in kernel space, some api in user space cannot be called.
	MS_BOOL status = true;
	MS_U32 config_Fc_reg = 0;
	MS_U32 Fc_over_Fs_reg = 0;
	MS_U16 Cfo_offset_reg = 0;
  MS_U32 f_Fc =0;// FreqCfo_offset;
  
 	MS_U16 snr_reg;

  DVBC_GetSNR_PARAM Drv_DVBC_GetSNR_PARAM;
  Drv_DVBC_GetSNR_PARAM.snr_reg=&snr_reg;
  Drv_DVBC_GetSNR_PARAM.ret = false;
  
  DVBC_GetStatus_PARAM Drv_DVBC_GetStatus_PARAM;
  Drv_DVBC_GetStatus_PARAM.pQAMMode=pQAMMode;
  Drv_DVBC_GetStatus_PARAM.u16SymbolRate=u16SymbolRate;
  Drv_DVBC_GetStatus_PARAM.config_Fc_reg=&config_Fc_reg;
  Drv_DVBC_GetStatus_PARAM.Fc_over_Fs_reg=&Fc_over_Fs_reg;
  Drv_DVBC_GetStatus_PARAM.Cfo_offset_reg=&Cfo_offset_reg; // FC
  Drv_DVBC_GetStatus_PARAM.ret=false;
  
  DVBC_GetLockWithRFPower Drv_DVBC_GetLockWithRFPower;
  Drv_DVBC_GetLockWithRFPower.eType=eType;
  Drv_DVBC_GetLockWithRFPower.eLockStatus=eLockStatus;
  Drv_DVBC_GetLockWithRFPower.u32TimeInterval=0;
  Drv_DVBC_GetLockWithRFPower.ret=false;
  
  if(u32DVBCopen==1)
  {
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetStatus,&Drv_DVBC_GetStatus_PARAM);
    UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetSNR,&Drv_DVBC_GetSNR_PARAM);
   
     f_Fc = Fc_over_Fs_reg/134217728 * 48000;
    *pFreqOff = f_Fc - (48000 - Cfo_offset_reg);

    if(snr_reg <= 6)  // kernel drvier cannot support log computation. Use table intstead.
    SNR = 42;
    else if(snr_reg <= 12)
    SNR = 39;
    else if(snr_reg <= 24)
    SNR = 36;
    else if(snr_reg <= 48)
    SNR = 33;
    else if(snr_reg <= 96)
    SNR = 30;
    else if(snr_reg <= 192)
    SNR = 27;
    else if(snr_reg <= 384)
    SNR = 24;
    else if(snr_reg <= 768)
    SNR = 21;
    else if(snr_reg <= 1536)
    SNR = 18;
    else if(snr_reg <= 3072)
    SNR = 15;
    else if(snr_reg <= 6144)
    SNR = 12;
    else if(snr_reg <= 12288)
    SNR = 9;               
    else if(snr_reg <= 24576)
    SNR = 6;
    else if(snr_reg <= 49152)
    SNR = 3;
    else
    SNR = 0;

    #ifdef MS_DEBUG
    if (_u8DMD_DVBC_DbgLevel >= DMD_DVBC_DBGLV_INFO)
    {
        ULOGD("DEMOD","MDrv_DMD_DVBC_GetMdbStatus %d %d %f\n", *pQAMMode, *u16SymbolRate, *pFreqOff);
    }
    #endif
  }
  else
  {
  	ULOGD("DEMOD","[drvDMD_INTERN_DVBC.c] MDrv_DMD_DVBC_GetMdbStatus: DVBC Open Fail\n");
    return false;
  }   

  #ifdef MS_DEBUG 
  //if(*(Drv_DVBC_GetLockWithRFPower_Transform.eLockStatus)==DMD_DVBC_LOCK)
  if(*(Drv_DVBC_GetLockWithRFPower.eLockStatus)==DMD_DVBC_LOCK)
  {
    ULOGD("DEMOD","check in drv layer DVBC demod locked!!\n");
  }
  else
  {
    ULOGD("DEMOD","check in drv layer DVBC demod unlock!!\n");
    return false;
  }
  #endif

  return status; 
}
#endif

MS_BOOL MDrv_DMD_DVBC_GetDEMODMdbInfo(MS_U64 *u64ReqHdl)
{
  // define the variable and struct here
    MS_BOOL status = true;
    MS_U16 SymbolRate = 0, i = 0;
    MS_U16 SQI = 0;
    MS_U16 pkterr = 0; 
    MS_U16 FWver;
    MS_U32 pData = 0, pkterr_sum = 0; // get sfo
    #ifndef MSOS_TYPE_LINUX_KERNEL
    float f_symb_offset = 0.0f;
    float fber = 0.0f;
    float f_snr = 0.0f;
    float FreqOff = 0.0f;
    float log_ber = 0.0f;
    #else
    MS_U32 f_symb_offset = 0;
    MS_U32 fber = 0;
    MS_U32 f_snr = 0;
    MS_U32 FreqOff = 0;
    MS_U32 log_ber = 0;
    #endif
    MS_U8 QAM_MDB = 0;
    MS_U16 SR_MDB = 0;
    MS_U32 IF_MDB = 0;
    MS_BOOL SpecInv_MDB = 0;
    MS_BOOL SerialTS_MDB = 0; 
    
    DMD_DVBC_MODULATION_TYPE Qam_mode = DMD_DVBC_QAM16;
    DMD_DVBC_LOCK_STATUS eLockStatus = DMD_DVBC_CHECKING;

    MS_U8 ifagc_reg; // high byte
    MS_U8 ifagc_reg_lsb; // low byte
    MS_U16 ifagc_err_reg;
   
    DVBC_GetIFAGC_PARAM Drv_DVBC_GetIFAGC_PARAM;
    Drv_DVBC_GetIFAGC_PARAM.ifagc_reg=&ifagc_reg;
    Drv_DVBC_GetIFAGC_PARAM.ifagc_reg_lsb=&ifagc_reg_lsb;
    Drv_DVBC_GetIFAGC_PARAM.ifagc_err_reg=&ifagc_err_reg;
    Drv_DVBC_GetIFAGC_PARAM.ret=false;

    DVBC_GetCurrentSymbolRateOffset_PARAM Drv_DVBC_GetCurrentSymbolRateOffset_PARAM;
    Drv_DVBC_GetCurrentSymbolRateOffset_PARAM.pData=&pData;
    Drv_DVBC_GetCurrentSymbolRateOffset_PARAM.ret=false;

    // Link the Info
    if(u32DVBCopen==1)
    {
    	  UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetIFAGC,&Drv_DVBC_GetIFAGC_PARAM);
    	  UtopiaIoctl(ppDVBCInstant,DMD_DVBC_DRV_CMD_GetCurrentSymbolRateOffset,&Drv_DVBC_GetCurrentSymbolRateOffset_PARAM);
    	  
    	 #ifndef MSOS_TYPE_LINUX_KERNEL    // When procfs implement in kernel space, it cannot call back user space.
       status &= MDrv_DMD_DVBC_GetSNR(&f_snr);
       status &= MDrv_DMD_DVBC_GetLock(DMD_DVBC_GETLOCK_PSYNC_LOCK, &eLockStatus);
       if(eLockStatus == DMD_DVBC_LOCK)
       {
    	   if(MDrv_DMD_DVBC_GetPostViterbiBer(&fber) == FALSE)
         {
            DMD_DBG(ULOGD("DEMOD","\nGetPostViterbiBer Fail!"));
            return false;
         }
       }
       status &= MDrv_DMD_DVBC_GetStatus(&Qam_mode, &SymbolRate, &FreqOff); 
       status &= MDrv_DMD_DVBC_GetSignalQuality(&SQI);
       //sfo = Reg*2^(-37)*FB/FS*1000000 (2^-28 * 1000000 = 0.003725)
       f_symb_offset = (float)((MS_S32)pData) * (0.000014552f) * (float)SymbolRate/48000;  //FS =48000
       #else
       status &= MDrv_DMD_DVBC_GetMdbStatus(&Qam_mode, &SymbolRate, &FreqOff, DMD_DVBC_GETLOCK_PSYNC_LOCK, &eLockStatus, &f_snr);
       f_symb_offset = ((MS_S32)pData) * 14552/1000000000 * SymbolRate/48000;  //FS =48000
       #endif
       status &= MDrv_DMD_DVBC_GetPacketErr(&pkterr);
       status &= MDrv_DMD_DVBC_GetFWVer(&FWver);       
       
       for(i = 0; i < 20; i++)
       {
       MsOS_DelayTask(100);  //obersve pkterr in 2s and every 100ms
       status &= MDrv_DMD_DVBC_GetPacketErr(&pkterr);
       pkterr_sum = pkterr_sum + pkterr;
       }  
           
    }
    else
    {
    	 ULOGD("DEMOD","[drvDMD_INTERN_DVBC.c] MDrv_DMD_DVBC_GetDEMODMdbInfo: DVBC Open Fail\n");
       return false;
    }
    MDrv_DMD_DVBC_GetMDBConfig(&SR_MDB,&QAM_MDB,&IF_MDB,&SpecInv_MDB,&SerialTS_MDB);
 
    // Printf the Info
    MdbPrint(u64ReqHdl,"---------MStar Demod DVBC Basic Info----------\n");
    MdbPrint(u64ReqHdl,"Spec           : DVBC\n");
    //MdbPrint(u64ReqHdl,"Version Number : 0x%02X, 0x%02X, 0x00\n",(FWver>>8)&0xFF, FWver&0xFF);
    MdbPrint(u64ReqHdl,"Version Number : 0x%x, 0x0%x, 0x00\n",(FWver>>8)&0xFF, FWver&0xFF);
    MdbPrint(u64ReqHdl,"QAM Mode       : %d-QAM\n",(16<<Qam_mode));  //(0,1,2,3,4) = (16,32,64,128,256)
    
    if(DMD_DVBC_QAMAUTO==QAM_MDB)
    MdbPrint(u64ReqHdl,"Auto QAM Detect: Enable\n");
    else 	
    MdbPrint(u64ReqHdl,"Auto QAM Detect: Disable\n"); 
    	
    MdbPrint(u64ReqHdl,"Symbol Rate    : %d kS/s\n",SymbolRate);   
    
    if(0==SR_MDB)
    MdbPrint(u64ReqHdl,"Auto SR Detect : Enable\n"); 
    else
    MdbPrint(u64ReqHdl,"Auto SR Detect : Disable\n"); 
    
    MdbPrint(u64ReqHdl,"IF             : %d kHz\n",IF_MDB);
      
    MdbPrint(u64ReqHdl,"---------MStar Demod DVBC Status Info---------\n");
    if(eLockStatus == DMD_DVBC_LOCK)
    MdbPrint(u64ReqHdl,"Lock Status    : Lock\n");
    else
    MdbPrint(u64ReqHdl,"Lock Status    : Unlock\n");  

    //MdbPrint(u64ReqHdl,"SNR            : %.1f dB\n",f_snr);  // MdbPrint doesn't support float
    MdbPrint(u64ReqHdl,"SNR            : %d.%d dB\n",(MS_U8)(f_snr), (MS_U8)((f_snr -((MS_U8)(f_snr)))*10));
    
        #ifndef MSOS_TYPE_LINUX_KERNEL // Can not support float in kernel
        #ifdef MSOS_TYPE_LINUX
          log_ber = (-1.0f)*log10f(1.0f/fber);
        #else
          log_ber = (-1.0f)*Log10Approx(1.0f/fber); // Log10Approx() provide 1~2^32 input range only
        #endif

    MdbPrint(u64ReqHdl,"BER            : 1e%d\n",(MS_S32)log_ber);
    MdbPrint(u64ReqHdl,"SQI            : %d\n",SQI);
        #endif
    MdbPrint(u64ReqHdl,"Packet Error   : %d\n",pkterr);
    MdbPrint(u64ReqHdl,"CFO            : %d kHz\n",(MS_S32)FreqOff);
    MdbPrint(u64ReqHdl,"SFO            : %d ppm\n",(MS_S32)f_symb_offset);
    MdbPrint(u64ReqHdl,"Gain Value     : 0x%x%x\n",ifagc_reg, ifagc_reg_lsb); 
    MdbPrint(u64ReqHdl,"Gain Error Mean: 0x%x\n",ifagc_err_reg); 
    
    //Quick detection: observation in 2 seconds, and return result   
    if( (pkterr_sum==0) && (eLockStatus == DMD_DVBC_LOCK) )
    MdbPrint(u64ReqHdl,"Demod Status   : [OK] Demod locks and no packet error within 2 seconds.\n");	
    else if( (pkterr_sum!=0) && (eLockStatus == DMD_DVBC_LOCK) )
    MdbPrint(u64ReqHdl,"Demod Status   : [NG] Demod locks but has packet error within 2 seconds.\n");	
    else
    MdbPrint(u64ReqHdl,"Demod Status   : [NG] Demod unlocks.\n");	   	
    
    return status;      
}

char* _MDrv_DMD_DVBC_Str_Tok(char* pstrSrc, char* pstrDes, char delimit)
{
    char *pstrRes = pstrSrc;

    *pstrDes = '\0';

    if(pstrSrc == NULL)
        return NULL;

    while(*pstrRes != '\0')
    {
        if((*pstrRes == delimit) || (*pstrRes == ' '))
            break;
        *pstrDes++ = *pstrRes++;
    }
    *pstrDes = '\0';

    return (pstrRes + 1);
}

int _MDrv_DMD_DVBC_StrToInt(char* pstrSrc)
{
    int iRes = 0;
    char *pstrRes = pstrSrc;

    while(*pstrRes != '\0')
    {
        iRes *= 10;
        iRes += (int)((*pstrRes++) - '0');
    }

    return iRes;
}

MS_BOOL MDrv_DMD_DVBC_SetMdb(MS_U64* pu64ReqHdl, MS_U32 u32CmdSize, char* pcmd)
{
    MS_U8 QAM_MDB = 0;
    MS_U16 SR_MDB = 0;
    MS_U32 IF_MDB = 0;
    MS_BOOL SpecInv_MDB = 0;
    MS_BOOL SerialTS_MDB = 0; 
    char* ptr = NULL;
    char strbuf[128] = {0};
    if(NULL == pcmd)
        return false;

    ptr = pcmd;

    MdbPrint(pu64ReqHdl,"---------MStar DEMOD DVBC Set Command---------\n");
    MdbPrint(pu64ReqHdl,"cmd: %s\n",ptr);

    if(strncmp(ptr, "ifvalue=", 8) == 0)  // compare the first 8 digit
    {
        MS_U32 ifvalue_input;
             
        // Capture the IF value
        ptr = pcmd + 8;
        ptr = _MDrv_DMD_DVBC_Str_Tok(ptr, strbuf, ';');
        ifvalue_input = _MDrv_DMD_DVBC_StrToInt(strbuf);
        
        MDrv_DMD_DVBC_GetMDBConfig(&SR_MDB,&QAM_MDB,&IF_MDB,&SpecInv_MDB,&SerialTS_MDB); 
        
        if(MDrv_DMD_DVBC_SetConfig(SR_MDB,QAM_MDB,ifvalue_input,SpecInv_MDB,SerialTS_MDB)==true)
        MdbPrint(pu64ReqHdl,"Set IF Value Successfully. IF Value = %d\n",ifvalue_input);
        else
        MdbPrint(pu64ReqHdl,"Fail to Set IF Value. \n");
        
        MDrv_DMD_DVBC_SetActive(1);     
        return true;
    }
    else if(strncmp(ptr, "SetQAM=", 7) == 0)  // compare the first 7 digit
    {
        MS_U32 qam_input;
        
        // Capture the IF value
        ptr = pcmd + 7;
        ptr = _MDrv_DMD_DVBC_Str_Tok(ptr, strbuf, ';');
        qam_input = _MDrv_DMD_DVBC_StrToInt(strbuf);

        MDrv_DMD_DVBC_GetMDBConfig(&SR_MDB,&QAM_MDB,&IF_MDB,&SpecInv_MDB,&SerialTS_MDB);  
    
        // if qam = 128, go AutoQAM flow. 0,1,2,3,4 = 16,32,64,128,256   	
    	  if((qam_input==0)||(qam_input==1)||(qam_input==2)||(qam_input==3)||(qam_input==4)||(qam_input==128)){
        if(MDrv_DMD_DVBC_SetConfig(SR_MDB,qam_input,IF_MDB,SpecInv_MDB,SerialTS_MDB)==true)
        MdbPrint(pu64ReqHdl,"Set QAM Mode Successfully. QAM Mode = %d-QAM\n",(16<<qam_input));
        else
        MdbPrint(pu64ReqHdl,"Fail to Set QAM Mode. \n");
        
        MDrv_DMD_DVBC_SetActive(1);   
        }
        else{
        MdbPrint(pu64ReqHdl,"Invalid Input QAM Mode. \n");
        }
          
        return true;    	
    }
    else if(strncmp(ptr, "SetSR=", 6) == 0)  // compare the first 6 digit
    {
        MS_U32 sr_input;
        
        // Capture the IF value
        ptr = pcmd + 6;
        ptr = _MDrv_DMD_DVBC_Str_Tok(ptr, strbuf, ';');
        sr_input = _MDrv_DMD_DVBC_StrToInt(strbuf);        	
        MDrv_DMD_DVBC_GetMDBConfig(&SR_MDB,&QAM_MDB,&IF_MDB,&SpecInv_MDB,&SerialTS_MDB); 
       // if SR = 0, go AutoSR flow.
        if(MDrv_DMD_DVBC_SetConfig(sr_input,QAM_MDB,IF_MDB,SpecInv_MDB,SerialTS_MDB)==true)
        MdbPrint(pu64ReqHdl,"Set Symbol Rate Successfully. Symbol Rate = %d kS/s\n",sr_input);
        else
        MdbPrint(pu64ReqHdl,"Fail to Set Symbol Rate. \n");
        
        MDrv_DMD_DVBC_SetActive(1);   
        return true;     	
    }   	
    else if(strncmp(ptr, "help", 4) == 0)
    {
        MdbPrint(pu64ReqHdl,"-------------------------- MStar Demod DVBC Help ------------------------\n");
        MdbPrint(pu64ReqHdl,"Please check whether the demod information is normal.\n");
        MdbPrint(pu64ReqHdl,"Parameter                       Descriptions\n");
        MdbPrint(pu64ReqHdl,"=========================================================================\n");
        MdbPrint(pu64ReqHdl,"Lock Status                     Demod Lock or Unlock\n");
        MdbPrint(pu64ReqHdl,"FW Version Number               Consist of three heximal number. e.g. 0x18, 0x00, 0x00\n");
        MdbPrint(pu64ReqHdl,"QAM Mode                        16/32/64/128/256-QAM. This should be identical to signal setting.\n");
        MdbPrint(pu64ReqHdl,"Auto QAM Detect                 Enable or Disable. If this is disabled, then this will use default.\n");
        MdbPrint(pu64ReqHdl,"Symbol Rate                     Support range: 2~7 MS/s. This should be identical to signal setting.\n");
        MdbPrint(pu64ReqHdl,"Auto SR Detect                  Enable or Disable. If this is disabled, then this will use default.\n");
        MdbPrint(pu64ReqHdl,"IF(Intermediate Frequency)      Set by driver. e.g. 5000kHz\n");
        MdbPrint(pu64ReqHdl,"SNR(Signal to Noise Ratio)      Normal range: 10~42dB\n");
        MdbPrint(pu64ReqHdl,"BER(Bit Error Rate)             Normal range: 1e-2~1e-7\n");	
        MdbPrint(pu64ReqHdl,"SQI(Signal Quality Indicator)   Normal range: 0~100. If demod unlocks, SQI should be zero.\n");
        MdbPrint(pu64ReqHdl,"Packet error                    Number of packet error. Normal range: 0~4096\n");
        MdbPrint(pu64ReqHdl,"CFO(Carrier Frequency Offset)   Normal range: -600kHz ~ +600kHz\n");	
        MdbPrint(pu64ReqHdl,"SFO(Sampling Frequency Offset)  Normal range: -60ppm ~ + 60ppm\n");
        MdbPrint(pu64ReqHdl,"Gain Value                      Range from 0x0000 to 0xFFFF. If this not close to min(0x0000)/max(0xFFFF), it's normal.\n");     
        MdbPrint(pu64ReqHdl,"Gain Error Mean                 This should be zero.\n");
        MdbPrint(pu64ReqHdl,"========================== Debug Suggestion =============================\n");
        MdbPrint(pu64ReqHdl,"1. Check whether demod locks or not and tuner status is normal.\n");
        MdbPrint(pu64ReqHdl,"2. Check the signal setting(QAM, symbol rate) is correct.\n");
        MdbPrint(pu64ReqHdl,"3. Check each parameter of demod status is within the normal range.\n");
        MdbPrint(pu64ReqHdl,"4. If SFO and Gain value is abnormal, it may relates to tuner. Double check the status of tuner\n");  
        MdbPrint(pu64ReqHdl,"5. Get the demod status info several times and observe the variation of each parameter.\n");        
        MdbPrint(pu64ReqHdl,"   If these parameters work normal, demod status also works normal. The TV display is \n");
        MdbPrint(pu64ReqHdl,"   still abnormal. You can check the next related module.\n");       
        return true;	  	
  	}
    else
    {
         MdbPrint(pu64ReqHdl,"[%d]DBG CMD Error!!!\n", __LINE__);
         return false;
    }
}
#endif