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
#define HAL_HVSP_C

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition

#include "drv_scl_os.h"
#include "hal_utility.h"
#include "hal_scl_reg.h"
#include "hal_scl_util.h"
#include "drv_scl_hvsp_st.h"
#include "drv_scl_irq_st.h"
#include "drv_scl_irq.h"
#include "hal_scl_hvsp.h"
#include "drv_scl_dbg.h"
#include "drv_scl_pq_define.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"
#include "Infinity_Iq.h"             // table config parameter
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HVSP_CMD_TRIG_BUFFER_SIZE 100
#define IsSc1InputSource(Src)             (gstGlobalHalHvspSet->genIpType[E_DRV_SCLHVSP_ID_1] == (Src))
#define IsClkIncrease(height,width,rate)    (((height) > 720 || (width) > 1280)&& (rate) < 172000000)
#define IsClkDecrease(height,width,rate)    (((height) <= 720 && (width) <= 1280)&& (rate) >= 172000000)
#define HAL_HVSP_RATIO(input, output)           ((u32)((u64)((u32)(input) * 1048576) / (u32)(output)))
#define DISABLE_CLK 0x1
#define LOW_CLK 0x4

#define HVSP_SRAM_SIZE_MLOAD ((PQ_IP_SRAM1_SIZE_Main/5)*8)
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MDrvSclCtxHalHvspGlobalSet_t *gstGlobalHalHvspSet;

//keep
u32 HVSP_RIU_BASE;
void *gpvScUpBuffer[4] = {NULL,NULL,NULL,NULL};
void *gpvScDownBuffer[4] = {NULL,NULL,NULL,NULL};
DrvSclOsDmemBusType_t gpvphyScUpBuffer[4] = {NULL,NULL,NULL,NULL};
DrvSclOsDmemBusType_t gpvphyScDownBuffer[4] = {NULL,NULL,NULL,NULL};
//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------
void _HalSclHvspSetGlobal(MDrvSclCtxCmdqConfig_t *pvCtx)
{
    gstGlobalHalHvspSet = &(((MDrvSclCtxGlobalSet_t*)(pvCtx->pgstGlobalSet))->stHalHvspCfg);
}

void HalSclHvspExit(void)
{
    void *pvCtx;
    u16 i;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    for(i=0;i<E_DRV_SCLHVSP_ID_MAX;i++)
    {
        gstGlobalHalHvspSet->genIpType[i] = 0;
    }
    gstGlobalHalHvspSet->gu16width[0]=0;
    gstGlobalHalHvspSet->gu16height[0]=0;
    gstGlobalHalHvspSet->gu16width[1]=0;
    gstGlobalHalHvspSet->gu16height[1]=0;
}
//============CLK================================
#if 0
void HalSclHvspSetIdClkOnOff(bool bEn,DrvSclHvspClkConfig_t* stclk)
{
    sclprintf("[hal]NO OF\n");
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    //I2 is pll mode
    HalUtilityW2BYTEMSK(REG_BLOCK_52_L,0x0100,0x0330);//hdiv_1st
    HalUtilityW2BYTEMSK(REG_BLOCK_53_L,0x0013,0xFF);//div_2nd
    HalUtilityW2BYTEMSK(REG_BLOCK_52_L,BIT0,BIT0);//reset
    HalUtilityW2BYTEMSK(REG_BLOCK_52_L,0x0000,BIT0);//reset
    HalUtilityW2BYTEMSK(REG_BLOCK_54_L,0x8011,0xFFFF);//h61
    HalUtilityW2BYTEMSK(REG_BLOCK_50_L,0x0010,0xFFFF);//h61
    HalUtilityW2BYTEMSK(REG_BLOCK_51_L,0x0000,0xFFFF);//h61
}
void HalSclHvspSetClkRate(u8 u8Idx)
{
    //I2 is pll mode
}

void HalSclHvspFclk1(DrvSclHvspClkConfig_t *stclk)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    if ((HalUtilityR2BYTEDirect(REG_SCL_CLK_51_L)&DISABLE_CLK))
    {
    }
    else
    {
        HalUtilityW2BYTEMSK(REG_SCL_CLK_51_L,0x0000,0x001F);//h61
    }

}

void HalSclHvspFclk2(DrvSclHvspClkConfig_t *stclk)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    if ((HalUtilityR2BYTEDirect(REG_SCL_CLK_51_L)&(DISABLE_CLK<<8)))
    {
    }
    else
    {
        HalUtilityW2BYTEMSK(REG_SCL_CLK_51_L,0x0000,0x1F00);//h61
    }
}
#endif

//===========================================================
void HalSclHvspSramDumpbyMload(DrvSclHvspIdType_e enID,u32 u32Sram,bool bHer,bool bchange)
{
    u32 u32Addr = 0;
    u32 u32Reg = 0;
    bool bup = 0;
    bool u8Tbl = 0;
    bool u8idx = 0;
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    u8idx = (enID==E_DRV_SCLHVSP_ID_1) ? E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H :
        (enID==E_DRV_SCLHVSP_ID_2) ? E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H_1 :
        (enID==E_DRV_SCLHVSP_ID_3) ? E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H_2 :
        (enID==E_DRV_SCLHVSP_ID_4) ? E_HAL_SCLHVSP_SRAM_MLOAD_HVSP_H_3 : 0;
    u32Reg = (enID==E_DRV_SCLHVSP_ID_1) ? REG_SCL_HVSP0_40_L :
        (enID==E_DRV_SCLHVSP_ID_2) ? REG_SCL_HVSP1_40_L :
        (enID==E_DRV_SCLHVSP_ID_3) ? REG_SCL_HVSP2_40_L :
        (enID==E_DRV_SCLHVSP_ID_4) ? REG_SCL_HVSP3_40_L : REG_SCL_HVSP0_40_L;
    if(!bHer)
    {
        u8idx++;
    }
    bup = u32Sram & 0x1;
    u8Tbl = ((u32Sram&0xF0)>>4);
    if(u8Tbl<=4)
    {
        if(bup)
        {
#if defined(SCLOS_TYPE_MLOAD)
            u32Addr = (u32)((u32)gpvScUpBuffer[u8Tbl]);
#else
            u32Addr = (u32)((u32)(gpvphyScUpBuffer[u8Tbl]));
#endif
        }
        else
        {
#if defined(SCLOS_TYPE_MLOAD)
            u32Addr = (u32)((u32)gpvScDownBuffer[u8Tbl]);
#else
            u32Addr = (u32)((u32)(gpvphyScDownBuffer[u8Tbl]));
#endif
        }
    }
    if(u32Addr==0)
    {
        return;
    }
    gstGlobalHalHvspSet->pvMloadBuf[u8idx] = (void *)u32Addr;
    gstGlobalHalHvspSet->u8MloadBufid[u8idx] = u32Sram;
    gstGlobalHalHvspSet->bMloadchange[u8idx] = bchange;
    HalUtilityW2BYTEMSK(u32Reg,0x5,0x5);// enable mload
    //u32Addr = (u32Addr>>4);
    //HalUtilityW2BYTEMSK(REG_SCL_MLOAD_0B_L,0xa302,0xFFFF);//B
    //HalUtilityW2BYTEMSK(REG_SCL_MLOAD_00_L,u32Id,0x1F);//0 by id
    //HalUtilityW2BYTEMSK(REG_SCL_MLOAD_03_L,(u32Addr&0xFFFF),0xFFFF);// 3
    //HalUtilityW2BYTEMSK(REG_SCL_MLOAD_04_L,((u32Addr>>16)&0x1FF),0xFFFF);// 4
    //HalUtilityW2BYTEMSK(REG_SCL_MLOAD_01_L,0x00ff,0xFFFF);// 1
    //HalUtilityW2BYTEMSK(REG_SCL_MLOAD_02_L,0x0000,0xFFFF);// 2
    //HalUtilityW2BYTEMSK(REG_SCL_MLOAD_09_L,0x0003,0xF);// 9
}
void HalSclHvspMloadSramBufferFree(void)
{
    u8 u8idx;
    if(gpvScUpBuffer[0] != NULL)
    {
        DrvSclOsDirectMemFree("HVSPSRAM",HVSP_SRAM_SIZE_MLOAD*8,gpvScUpBuffer[0],gpvphyScUpBuffer[0]);
        //DrvSclOsVirMemFree(gpvScUpBuffer[0]);
        for(u8idx=0;u8idx<4;u8idx++)
        {
            gpvScUpBuffer[u8idx] = NULL;
            gpvScDownBuffer[u8idx] = NULL;
            gpvphyScUpBuffer[u8idx] = NULL;
            gpvphyScDownBuffer[u8idx] = NULL;
        }
    }
    HalUtilityW2BYTEMSK(REG_SCL_HVSP0_40_L,0,0x5);// disable mload
    HalUtilityW2BYTEMSK(REG_SCL_HVSP1_40_L,0,0x5);// disable mload
    HalUtilityW2BYTEMSK(REG_SCL_HVSP2_40_L,0,0x5);// disable mload
    HalUtilityW2BYTEMSK(REG_SCL_HVSP3_40_L,0,0x5);// disable mload
}
void HalSclHvspMloadSramBufferPrepare(void)
{
    // size= (320/5)(entry)*8(byte)=512
    u8 u8idx;
    u16 u16tcount;
    u16 u16addr;
    u64 u64val;
    u64 *p64UpBuffer = NULL;
    u64 *p64DownBuffer = NULL;
    if(gpvScUpBuffer[0])
    {
        //SCL_ERR("[%s]Aleady!!!\n",__FUNCTION__);
        return;
    }
    gpvScUpBuffer[0] = DrvSclOsDirectMemAlloc("HVSPSRAM",HVSP_SRAM_SIZE_MLOAD*8,&gpvphyScUpBuffer[0]);
    //gpvScUpBuffer[0] = DrvSclOsVirMemalloc(HVSP_SRAM_SIZE_MLOAD*8);
    if(gpvScUpBuffer[0]==NULL)
    {
        SCL_ERR("[%s]Fail!!!\n",__FUNCTION__);
        return;
    }
    for(u8idx=0;u8idx<4;u8idx++)
    {
        gpvScUpBuffer[u8idx] = gpvScUpBuffer[0]+(u8idx*2)*HVSP_SRAM_SIZE_MLOAD;
        gpvScDownBuffer[u8idx] = gpvScUpBuffer[u8idx]+HVSP_SRAM_SIZE_MLOAD;
        gpvphyScUpBuffer[u8idx] = gpvphyScUpBuffer[0]+(u8idx*2)*HVSP_SRAM_SIZE_MLOAD;
        gpvphyScDownBuffer[u8idx] = gpvphyScUpBuffer[u8idx]+HVSP_SRAM_SIZE_MLOAD;
    }
    for(u8idx=0;u8idx<4;u8idx++)
    {
        u16tcount = 0;
        p64UpBuffer = (u64 *)gpvScUpBuffer[u8idx];
        p64DownBuffer = (u64 *)gpvScDownBuffer[u8idx];
        //SCL_ERR("[%s]p64UpBuffer:%lx p64DownBuffer:%lx \n",__FUNCTION__,(u32)p64UpBuffer,(u32)p64DownBuffer);
        for(u16addr=0;u16addr<(PQ_IP_SRAM1_SIZE_Main/5);u16addr++)
        {
            u64val = ((u64)MST_SRAM1_Main[u8idx][u16tcount]|((u64)MST_SRAM1_Main[u8idx][u16tcount+1]<<8)|
                ((u64)MST_SRAM1_Main[u8idx][u16tcount+2]<<16)|((u64)MST_SRAM1_Main[u8idx][u16tcount+3]<<24)|
                ((u64)MST_SRAM1_Main[u8idx][u16tcount+4]<<32));
            if(p64UpBuffer)
            {
                *(p64UpBuffer+u16addr) = u64val;
            }
            u64val = ((u64)MST_SRAM2_Main[u8idx][u16tcount]|((u64)MST_SRAM2_Main[u8idx][u16tcount+1]<<8)|
                ((u64)MST_SRAM2_Main[u8idx][u16tcount+2]<<16)|((u64)MST_SRAM2_Main[u8idx][u16tcount+3]<<24)|
                ((u64)MST_SRAM2_Main[u8idx][u16tcount+4]<<32));
            if(p64DownBuffer)
            {
                *(p64DownBuffer+u16addr) = u64val;
            }
            u16tcount+=5;
        }
    }
    DrvSclOsDirectMemFlush((u32)gpvScUpBuffer[0],HVSP_SRAM_SIZE_MLOAD*8);
}


void HalSclHvspSetRiuBase(u32 u32RiuBase)
{
    HVSP_RIU_BASE = u32RiuBase;
}
void HalSclHvspSetInputSrcSize(ST_HVSP_SIZE_CONFIG *stSrc)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    DrvSclOsMemcpy(&gstGlobalHalHvspSet->gstHalSrcSize,stSrc,sizeof(ST_HVSP_SIZE_CONFIG));
}
void HalSclHvspReSetHw(void *pvCtx)
{
    HalUtilityW2BYTEMSKDirect(REG_VIP_MCNR_01_L, 0, BIT0);//IPM
    HalUtilityW2BYTEMSKDirect(REG_UCM1_03_L, 0, BIT2); //prv_crop
}

void HalSclHvspSetReset(void *pvCtx)
{
    _HalSclHvspSetGlobal(pvCtx);
    HalSclHvspSetNlmEn(0);
    HalUtilityW2BYTEMSK(REG_SCL2_50_L, 0, BIT0);//crop1
    HalUtilityW2BYTEMSK(REG_SCL2_58_L, 0, BIT0);//crop2
    HalUtilityW2BYTEMSK(REG_VIP_MCNR_01_L, 0, BIT0);//IPM
}
void HalSclHvspSetUcmClk(void)
{
    HalUtilityW2BYTEMSK(REG_UCM1_03_L,(BIT8|BIT9|BIT14),0x7F00);
}
void HalSclHvspSetVipSize(u16 u16Width, u16 u16Height)
{
    HalUtilityW2BYTEMSK(REG_VIP_LCE_6E_L, u16Width, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_VIP_LCE_6F_L, u16Height, 0x1FFF);
}
void HalSclHvspSetWdrGlobalSize(u16 u16Width, u16 u16Height)
{
    HalUtilityW2BYTEMSK(REG_VIP_WDR1_0B_L, u16Width-1, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_VIP_WDR1_0C_L, u16Height-1, 0x1FFF);
}
void HalSclHvspSetWdrLocalSize(u16 u16Width, u16 u16Height)
{
    u16 u16temp;
    HalUtilityW2BYTEMSK(REG_VIP_WDR1_0B_L, u16Width-1, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_VIP_WDR1_0C_L, u16Height-1, 0x1FFF);
    if(u16Width >=u16Height)
    {
        //box num 16:9 or 16:12
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_08_L, 15, 0x00FF);
        u16temp = ((u16Height+((u16Width+15)/16)-1)/((u16Width+15)/16))-1;
        if(u16temp<8)
        {
            u16temp = 8;
        }
        else if(u16temp>15)
        {
            u16temp = 15;
        }
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_08_L, u16temp<<8, 0xFF00);
        //box w  h = (WH+box-1)/box
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_06_L, ((u16Width+15)/16), 0x1FFF);
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_07_L, ((u16Height+u16temp)/(u16temp+1)), 0x1FFF);
        //reci w h = 65536/boxwh
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_09_L, 65536/((u16Width+15)/16), 0x1FFF);
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_0A_L, 65536/((u16Height+u16temp)/(u16temp+1)), 0x1FFF);
        SCL_DBG(SCL_DBG_LV_DRVHVSP(),"[HVSPWDR]box num %hu box w h %hu %hu reci w h %hu %hu\n"
            ,u16temp,((u16Width+15)/16),((u16Height+u16temp)/(u16temp+1)),65536/((u16Width+15)/16),65536/((u16Height+u16temp)/(u16temp+1)));
    }
    else
    {
        //box num 16:9 or 16:12
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_08_L, 15<<8, 0xFF00);
        u16temp = ((u16Width+((u16Height+15)/16)-1)/((u16Height+15)/16))-1;
        if(u16temp<8)
        {
            u16temp = 8;
        }
        else if(u16temp>15)
        {
            u16temp = 15;
        }
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_08_L, u16temp, 0xFF);
        //box w  h = (WH+box-1)/box
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_06_L, ((u16Width+u16temp)/(u16temp+1)), 0x1FFF);
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_07_L, ((u16Height+15)/16), 0x1FFF);
        //reci w h = 65536/boxwh
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_09_L, 65536/((u16Width+u16temp)/(u16temp+1)), 0x1FFF);
        HalUtilityW2BYTEMSK(REG_VIP_WDR1_0A_L, 65536/((u16Height+15)/16), 0x1FFF);
        SCL_DBG(SCL_DBG_LV_DRVHVSP(),"[HVSPWDR]box num %hu box w h %hu %hu reci w h %hu %hu\n"
            ,u16temp,((u16Width+u16temp)/(u16temp+1)),((u16Height+15)/16),65536/((u16Width+u16temp)/(u16temp+1)),65536/((u16Height+15)/16));
    }
}
void HalSclHvspSetMXnrSize(u16 u16Width, u16 u16Height)
{
    HalUtilityW2BYTEMSK(REG_VIP_SCNR_0A_L, u16Width -1, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_VIP_SCNR_0B_L, u16Height -1, 0x1FFF);
}
void HalSclHvspSetUVadjSize(u16 u16Width, u16 u16Height)
{
    HalUtilityW2BYTEMSK(REG_VIP_SCNR_3B_L, u16Width-1, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_VIP_SCNR_3C_L, u16Height-1, 0x1FFF);
}
void HalSclHvspSetXnrSize(u16 u16Width, u16 u16Height)
{
    HalUtilityW2BYTEMSK(REG_VIP_SCNR_5C_L, u16Width-1, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_VIP_SCNR_5D_L, u16Height-1, 0x1FFF);
}
void HalSclHvspSetSWReset(bool bEn)
{
    HalUtilityW2BYTEMSK(REG_SCL0_01_L, bEn ? BIT0 : 0, BIT0);
    HalUtilityW2BYTEMSK(REG_SCL0_01_L, 0, BIT0);
}
void HalSclHvspSetDNRReset(bool bEn)
{
    // for issue reset rstz_ctrl_clk_miu_sc_dnr cause to overwrite 0x0
    //HalUtilityW2BYTEMSK(REG_SCL_DNR1_24_L, bEn ? BIT6 : 0, BIT6);
    //HalUtilityW2BYTEMSK(REG_SCL_DNR1_60_L, bEn ? BIT0 : 0, BIT0);
}

void HalSclHvspSetNlmLineBufferSize(u16 u16Width, u16 u16Height)
{
    HalUtilityW2BYTEMSK(REG_SCL0_19_L, 0x8000, 0x8000);
    HalUtilityW2BYTEMSK(REG_SCL0_1A_L, u16Width, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_SCL0_1B_L, u16Height, 0x1FFF);
}

void HalSclHvspSetNlmEn(bool bEn)
{
    HalUtilityW2BYTEMSK(REG_SCL0_18_L, bEn ? BIT0 : 0, BIT0);
}


//-------------------------------------------------------------------------------------------------
// Crop
//-------------------------------------------------------------------------------------------------
void HalSclHvspSetNlmRegionSize(HalSclHvspCropInfo_t *stCropInfo)
{
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_05_L, stCropInfo->bEn ? BIT7 : 0, BIT7);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_06_L, stCropInfo->u16Hsize, 0x3FFF);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_07_L, stCropInfo->u16Vsize, 0x3FFF);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_08_L, stCropInfo->u16Hst, 0x3FFF);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_09_L, stCropInfo->u16Vst, 0x3FFF);
}

void HalSclHvspSetCropConfig(DrvSclHvspCropIdType_e enID, HalSclHvspCropInfo_t *stCropInfo)
{
    u32 u32reg_idx = enID == E_DRV_SCLHVSP_CROP_ID_1 ? 0x00 : 0x10;

    //Crop01: 0x50
    //Crop02: 0x58
    if((stCropInfo->u16In_hsize == stCropInfo->u16Hsize) && (stCropInfo->u16Vsize == stCropInfo->u16In_vsize))
    {
        HalUtilityW2BYTEMSK(REG_SCL2_50_L + u32reg_idx, 0, BIT0);
    }
    else
    {
        HalUtilityW2BYTEMSK(REG_SCL2_50_L + u32reg_idx, stCropInfo->bEn ? BIT0 : 0, BIT0);
    }
    HalUtilityW2BYTEMSK(REG_SCL2_51_L + u32reg_idx, stCropInfo->u16In_hsize, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_SCL2_52_L + u32reg_idx, stCropInfo->u16In_vsize, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_SCL2_53_L + u32reg_idx, stCropInfo->u16Hst, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_SCL2_54_L + u32reg_idx, stCropInfo->u16Hsize, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_SCL2_55_L + u32reg_idx, stCropInfo->u16Vst, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_SCL2_56_L + u32reg_idx, stCropInfo->u16Vsize, 0x1FFF);
}

//-------------------------------------------------------------------------------------------------
// IP Mux
//-------------------------------------------------------------------------------------------------
void HalSclHvspSetInputMuxType(DrvSclHvspIdType_e enID,DrvSclHvspIpMuxType_e enIpType)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    gstGlobalHalHvspSet->genIpType[enID] = enIpType;
}

u32 HalSclHvspGetInputSrcMux(DrvSclHvspIdType_e enID)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    return gstGlobalHalHvspSet->genIpType[enID];
}

void HalSclHvspSetHwSc3InputMux(DrvSclHvspIpMuxType_e enIpType,void *pvCtx)
{
    u16 u16Val = 0;
    if(enIpType==E_DRV_SCLHVSP_IP_MUX_HVSP)
    {
        u16Val = BIT1;
    }
    else if(enIpType==E_DRV_SCLHVSP_IP_MUX_RSC)
    {
        u16Val = 0;
    }
    else if(enIpType==E_DRV_SCLHVSP_IP_MUX_LDC)
    {
        u16Val = BIT0;
    }
    HalUtilityW2BYTEMSK(REG_SCL_RSC_10_L, u16Val, 0x3);
}


//-------------------------------------------------------------------------------------------------
// IPM
//-------------------------------------------------------------------------------------------------
void HalSclHvspSetIpmYCMReadEn(bool bEn)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    HalUtilityW2BYTEMSK(REG_UCM1_03_L, bEn ? 0 : BIT1, BIT1);
}

void HalSclHvspSetIpmYCMWriteEn(bool bEn)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    HalUtilityW2BYTEMSK(REG_VIP_MCNR_01_L, bEn ? BIT0 : 0, BIT0);
}
void HalSclHvspSetUcmYCBase1(u32 u32Base)
{
    u16 u16Base_Lo, u16Base_Hi;
    u32Base = u32Base>>5;
    u16Base_Lo = u32Base & 0xFFFF;
    u16Base_Hi = (u32Base & 0x0FFF0000) >> 16;
    HalUtilityW2BYTE(REG_UCM1_08_L, u16Base_Lo);
    HalUtilityW2BYTE(REG_UCM1_09_L, u16Base_Hi);
}
void HalSclHvspSetUcmYCBase2(u32 u32Base)
{
    u16 u16Base_Lo, u16Base_Hi;
    u32Base = u32Base>>5;
    u16Base_Lo = u32Base & 0xFFFF;
    u16Base_Hi = (u32Base & 0x0FFF0000) >> 16;
    HalUtilityW2BYTE(REG_UCM1_0A_L, u16Base_Lo);
    HalUtilityW2BYTE(REG_UCM1_0B_L, u16Base_Hi);
}
void HalSclHvspSetUCMConpress(HalSclUcmOnOffType_e enType)
{
    u16 u16ce8;
    u16 u16ce6;
    u16 u16cebypass;
    switch(enType)
    {
        case E_HAL_SCLUCM_CE8_ON:
            u16ce8 = BIT0;
            u16ce6 = 0;
            u16cebypass = BIT8;
            break;
        case E_HAL_SCLUCM_CE6_ON:
            u16ce8 = 0;
            u16ce6 = BIT0;
            u16cebypass = BIT0;
            break;
        case E_HAL_SCLUCM_OFF:
            u16ce8 = 0;
            u16ce6 = 0;
            u16cebypass = BIT0|BIT8;
            break;
        default:
            u16ce8 = 0;
            u16ce6 = 0;
            u16cebypass = BIT0|BIT8;
            break;
    }
    HalUtilityW2BYTEMSK(REG_UCM2_01_L, u16ce8, BIT0);//conpress
    HalUtilityW2BYTEMSK(REG_UCM2_08_L, u16ce6, BIT0);//decon
    HalUtilityW2BYTEMSK(REG_UCM2_10_L, u16cebypass, BIT0|BIT8);//conpress
}
void HalSclHvspSetIpmvSize(u16 u16Vsize)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    HalUtilityW2BYTEMSK(REG_UCM1_18_L, u16Vsize, 0x1FFF);
    HalUtilityW2BYTEMSK(REG_UCM1_18_L, BIT15, BIT15);
    gstGlobalHalHvspSet->gu16height[0] = u16Vsize;
}

void HalSclHvspSetIpmLineOffset(u16 u16Lineoffset)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    HalUtilityW2BYTEMSK(REG_UCM1_0E_L, u16Lineoffset, 0x1FFF);
    gstGlobalHalHvspSet->gu16width[0] = u16Lineoffset;
}
void HalSclHvspSetNeSampleStep(u16 u16Hsize,u16 u16Vsize)
{
	// (width/step) * (height/step) < target
	u32 u32sub_w, u32sub_h, u32sum_wh, u32step = 1,u32target;
    u32target = HalUtilityR2BYTE(REG_VIP_NE_10_L);
	for (u32step = 1; u32step < 32; u32step++)
	{
		u32sub_w = u16Hsize / u32step;
		u32sub_h = u16Vsize / u32step;
		u32sum_wh = u32sub_w * u32sub_h;

		if (u32sum_wh <= u32target)
		{
			break;
		}
	}
    SCL_DBG(SCL_DBG_LV_DRVHVSP()&EN_DBGMG_HVSPLEVEL_ELSE,"[%s]:%lu\n",__FUNCTION__,u32step);
    HalUtilityW2BYTEMSK(REG_VIP_NE_11_L, u32step, 0x1F);
}

void HalSclHvspSetIpmFetchNum(u16 u16FetchNum)
{
    HalUtilityW2BYTEMSK(REG_UCM1_0F_L, u16FetchNum, 0x1FFF);
}
void HalSclHvspSetIpmBufferNumber(u8 u8Num)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    HalUtilityW2BYTEMSK(REG_UCM1_01_L, (u8Num==1) ? BIT12 : 0 , BIT12);
}
void HalSclHvspSetUcmHWrwDiff(u16 u8Val)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    HalUtilityW2BYTEMSK(REG_UCM1_01_L, (u8Val==1) ?  0 : BIT11 , BIT11);
}
void HalSclHvspSetUcmMemConfig(HalSclUcmOnOffType_e enType)
{
    void *pvCtx;
    u16 u16Cfg = 0;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    u16Cfg |= BIT5|BIT1; // YUV420 en:bit5 motion bit 1
    if(enType&E_HAL_SCLUCM_YONLY_ON)
    {
        u16Cfg |= BIT4;
    }
    if(enType&E_HAL_SCLUCM_CE6_ON)
    {
        u16Cfg |= BIT3|BIT2;
    }
    HalUtilityW2BYTEMSK(REG_UCM1_02_L, u16Cfg , 0x3F);
}

//-------------------------------------------------------------------------------------------------
// Scaling
//-------------------------------------------------------------------------------------------------

void HalSclHvspSetScalingVeEn(DrvSclHvspIdType_e enID, bool ben)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0A_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0A_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0A_L :
                                           REG_SCL_HVSP0_0A_L;

    HalUtilityW2BYTEMSK(u32reg, ben ? BIT8 : 0, BIT8);
}
void HalSclHvspSetScalingVeFactor(DrvSclHvspIdType_e enID, u32 u32Ratio)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_09_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_09_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_09_L :
                                           REG_SCL_HVSP0_09_L;

    HalUtilityW2BYTEMSK(u32reg, u32Ratio & 0xFFFF, 0xFFFF);
    HalUtilityW2BYTEMSK(u32reg+2, (u32Ratio >> 16) & 0x00FF, 0x00FF);
}

void HalSclHvspSetScalingHoEn(DrvSclHvspIdType_e enID, bool ben)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_08_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_08_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_08_L :
                                          REG_SCL_HVSP0_08_L;

    HalUtilityW2BYTEMSK(u32reg, ben ? BIT8 : 0, BIT8);
}

void HalSclHvspSetScalingHoFacotr(DrvSclHvspIdType_e enID, u32 u32Ratio)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_07_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_07_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_07_L :
                                          REG_SCL_HVSP0_07_L;

    HalUtilityW2BYTEMSK(u32reg, u32Ratio & 0xFFFF, 0xFFFF);
    HalUtilityW2BYTEMSK(u32reg+2, (u32Ratio >> 16) & 0x00FF, 0x00FF);
}
void HalSclHvspSetModeYHo(DrvSclHvspIdType_e enID, HalSclHvspFilterMode_e enFilterMode)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0B_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0B_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0B_L :
                                           REG_SCL_HVSP0_0B_L;
    HalUtilityW2BYTEMSK(u32reg, enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_BYPASS ? 0 : BIT0, BIT0);
    HalUtilityW2BYTEMSK(u32reg, (enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_SRAM_0 || enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_SRAM_1) ? BIT6 : 0, BIT6);
    HalUtilityW2BYTEMSK(u32reg, enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_SRAM_1 ? BIT7 : 0, BIT7);
}

void HalSclHvspSetModeYVe(DrvSclHvspIdType_e enID, HalSclHvspFilterMode_e enFilterMode)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0B_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0B_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0B_L :
                                           REG_SCL_HVSP0_0B_L;

    HalUtilityW2BYTEMSK(u32reg, enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_BYPASS ? 0 : BIT8, BIT8);
    HalUtilityW2BYTEMSK(u32reg, (enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_SRAM_0 || enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_SRAM_1) ? BIT14 : 0, BIT14);
    HalUtilityW2BYTEMSK(u32reg, enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_SRAM_1 ? BIT15 : 0, BIT15);
}

void HalSclHvspSetModeCHo(DrvSclHvspIdType_e enID, HalSclHvspFilterMode_e enFilterMode, HalSclHvspSramSelType_e enSramSel)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0B_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0B_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0B_L :
                                           REG_SCL_HVSP0_0B_L;

    if(enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_BYPASS || enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_BILINEAR)
    {
        HalUtilityW2BYTEMSK(u32reg, enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_BYPASS ? (0<<1) : (1<<1), (BIT3|BIT2|BIT1));
        HalUtilityW2BYTEMSK(u32reg, 0x0000, BIT4); // ram_en
        HalUtilityW2BYTEMSK(u32reg, enSramSel == E_HAL_SCLHVSP_SRAM_SEL_0 ? (0<<5) : (1<<5), BIT5); //ram_sel
    }
    else
    {
        HalUtilityW2BYTEMSK(u32reg, enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_SRAM_0 ? (2<<1) : (3<<1), (BIT3|BIT2|BIT1));
        HalUtilityW2BYTEMSK(u32reg, BIT4, BIT4); // ram_en
        HalUtilityW2BYTEMSK(u32reg, enSramSel == E_HAL_SCLHVSP_SRAM_SEL_0 ? (0<<5) : (1<<5), BIT5); //ram_sel
    }
}

void HalSclHvspSetModeCVe(DrvSclHvspIdType_e enID, HalSclHvspFilterMode_e enFilterMode, HalSclHvspSramSelType_e enSramSel)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0B_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0B_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0B_L :
                                           REG_SCL_HVSP0_0B_L;
    if(enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_BYPASS || enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_BILINEAR)
    {
        HalUtilityW2BYTEMSK(u32reg, enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_BYPASS ? (0<<9) : (1<<9), (BIT11|BIT10|BIT9));
        HalUtilityW2BYTEMSK(u32reg, 0x0000, BIT12); // ram_en
        HalUtilityW2BYTEMSK(u32reg, enSramSel == E_HAL_SCLHVSP_SRAM_SEL_0 ? (0<<13) : (1<<13), BIT13); //ram_sel
    }
    else
    {
        HalUtilityW2BYTEMSK(u32reg, enFilterMode == E_HAL_SCLHVSP_FILTER_MODE_SRAM_0 ? (2<<9) : (3<<9), (BIT11|BIT10|BIT9));
        HalUtilityW2BYTEMSK(u32reg, BIT12, BIT12); // ram_en
        HalUtilityW2BYTEMSK(u32reg, enSramSel == E_HAL_SCLHVSP_SRAM_SEL_0 ? (0<<13) : (1<<13), BIT13); //ram_sel
    }
}

void HalSclHvspSetHspDithEn(DrvSclHvspIdType_e enID, bool bEn)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0C_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0C_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0C_L :
                                           REG_SCL_HVSP0_0C_L;

    HalUtilityW2BYTEMSK(u32reg, bEn ? (BIT0) : (0), BIT0);
}


void HalSclHvspSetVspDithEn(DrvSclHvspIdType_e enID, bool bEn)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0C_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0C_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0C_L :
                                           REG_SCL_HVSP0_0C_L;

    HalUtilityW2BYTEMSK(u32reg, bEn ? (BIT1) : (0), BIT1);
}

void HalSclHvspSetHspCoringEnC(DrvSclHvspIdType_e enID, bool bEn)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0C_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0C_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0C_L :
                                           REG_SCL_HVSP0_0C_L;

    HalUtilityW2BYTEMSK(u32reg, bEn ? (BIT8) : (0), BIT8);
}

void HalSclHvspSetHspCoringEnY(DrvSclHvspIdType_e enID, bool bEn)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0C_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0C_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0C_L :
                                           REG_SCL_HVSP0_0C_L;
    HalUtilityW2BYTEMSK(u32reg, bEn ? (BIT9) : (0), BIT9);
}

void HalSclHvspSetVspCoringEnC(DrvSclHvspIdType_e enID, bool bEn)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0C_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0C_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0C_L :
                                           REG_SCL_HVSP0_0C_L;

    HalUtilityW2BYTEMSK(u32reg, bEn ? (BIT10) : (0), BIT10);
}

void HalSclHvspSetVspCoringEnY(DrvSclHvspIdType_e enID, bool bEn)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0C_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0C_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0C_L :
                                           REG_SCL_HVSP0_0C_L;
    HalUtilityW2BYTEMSK(u32reg, bEn ? (BIT11) : (0), BIT11);
}

void HalSclHvspSetHspCoringThrdC(DrvSclHvspIdType_e enID, u16 u16Thread)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0D_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0D_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0D_L :
                                           REG_SCL_HVSP0_0D_L;

    HalUtilityW2BYTEMSK(u32reg, u16Thread, 0x00FF);
}

void HalSclHvspSetHspCoringThrdY(DrvSclHvspIdType_e enID, u16 u16Thread)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0D_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0D_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0D_L :
                                           REG_SCL_HVSP0_0D_L;

    HalUtilityW2BYTEMSK(u32reg, u16Thread<<8, 0xFF00);
}

void HalSclHvspSetVspCoringThrdC(DrvSclHvspIdType_e enID, u16 u16Thread)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0E_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0E_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0E_L :
                                           REG_SCL_HVSP0_0E_L;

    HalUtilityW2BYTEMSK(u32reg, u16Thread, 0x00FF);
}

void HalSclHvspSetVspCoringThrdY(DrvSclHvspIdType_e enID, u16 u16Thread)
{
    u32 u32reg = enID == E_DRV_SCLHVSP_ID_2 ? REG_SCL_HVSP1_0E_L :
                    enID == E_DRV_SCLHVSP_ID_3 ? REG_SCL_HVSP2_0E_L :
                        enID == E_DRV_SCLHVSP_ID_4 ? REG_SCL_HVSP3_0E_L :
                                           REG_SCL_HVSP0_0E_L;

    HalUtilityW2BYTEMSK(u32reg, u16Thread<<8, 0xFF00);
}

void HalSclHvspSetHVSPInputSize(DrvSclHvspIdType_e enID, u16 u16Width, u16 u16Height)
{
    u32 u32reg_20, u32reg_21;
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    if(enID == E_DRV_SCLHVSP_ID_2)
    {
        u32reg_20 = REG_SCL_HVSP1_20_L;
        u32reg_21 = REG_SCL_HVSP1_21_L;
        gstGlobalHalHvspSet->gu16height[0] = (gstGlobalHalHvspSet->gu16height[0]<u16Height) ? u16Height : gstGlobalHalHvspSet->gu16height[0];
        gstGlobalHalHvspSet->gu16width[0] = (gstGlobalHalHvspSet->gu16width[0]<u16Width) ? u16Width : gstGlobalHalHvspSet->gu16width[0];
    }
    else if(enID == E_DRV_SCLHVSP_ID_3)
    {
        u32reg_20 = REG_SCL_HVSP2_20_L;
        u32reg_21 = REG_SCL_HVSP2_21_L;
        gstGlobalHalHvspSet->gu16height[1] = u16Height;
        gstGlobalHalHvspSet->gu16width[1] = u16Width;
    }
    else if(enID == E_DRV_SCLHVSP_ID_4)
    {
        u32reg_20 = REG_SCL_HVSP3_20_L;
        u32reg_21 = REG_SCL_HVSP3_21_L;
        gstGlobalHalHvspSet->gu16height[1] = u16Height;
        gstGlobalHalHvspSet->gu16width[1] = u16Width;
    }
    else
    {
        u32reg_20 = REG_SCL_HVSP0_20_L;
        u32reg_21 = REG_SCL_HVSP0_21_L;
        gstGlobalHalHvspSet->gu16height[0] =  (gstGlobalHalHvspSet->gu16height[0]<u16Height) ? u16Height : gstGlobalHalHvspSet->gu16height[0];
        gstGlobalHalHvspSet->gu16width[0] =  (gstGlobalHalHvspSet->gu16width[0]<u16Width) ? u16Width : gstGlobalHalHvspSet->gu16width[0];
    }

    HalUtilityW2BYTEMSK(u32reg_20, u16Width,  0xFFFF);
    HalUtilityW2BYTEMSK(u32reg_21, u16Height, 0xFFFF);
    if(enID ==E_DRV_SCLHVSP_ID_3 )
    {
        if(HalUtilityR2BYTE(REG_SCL_DMA1_32_L) != u16Width)
            HalUtilityW2BYTEMSK(REG_SCL_DMA1_32_L, u16Width, 0xFFFF);
        if(HalUtilityR2BYTE(REG_SCL_DMA1_33_L) != u16Height)
            HalUtilityW2BYTEMSK(REG_SCL_DMA1_33_L, u16Height,0xFFFF);
    }
}

void HalSclHvspSetHVSPOutputSize(DrvSclHvspIdType_e enID, u16 u16Width, u16 u16Height)
{
    u32 u32reg_22, u32reg_23;
    u32 u32ratio;
    u16 u16val_L,u16val_H;
    void *pvCtx;
    u64 u64Temp,u64Temp2;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _HalSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    if(enID == E_DRV_SCLHVSP_ID_2)
    {
        u32reg_22 = REG_SCL_HVSP1_22_L;
        u32reg_23 = REG_SCL_HVSP1_23_L;
        gstGlobalHalHvspSet->gu16height[0] = (gstGlobalHalHvspSet->gu16height[0]<u16Height) ? u16Height : gstGlobalHalHvspSet->gu16height[0];;
        gstGlobalHalHvspSet->gu16width[0] = (gstGlobalHalHvspSet->gu16width[0]<u16Width) ? u16Width : gstGlobalHalHvspSet->gu16width[0];
    }
    else if(enID == E_DRV_SCLHVSP_ID_3)
    {
        u32reg_22 = REG_SCL_HVSP2_22_L;
        u32reg_23 = REG_SCL_HVSP2_23_L;
        gstGlobalHalHvspSet->gu16height[1] =(gstGlobalHalHvspSet->gu16height[1]<u16Height) ? u16Height : gstGlobalHalHvspSet->gu16height[1];
        gstGlobalHalHvspSet->gu16width[1] = (gstGlobalHalHvspSet->gu16width[1]<u16Width) ? u16Width : gstGlobalHalHvspSet->gu16width[1];
    }
    else if(enID == E_DRV_SCLHVSP_ID_4)
    {
        u32reg_22 = REG_SCL_HVSP3_22_L;
        u32reg_23 = REG_SCL_HVSP3_23_L;
        gstGlobalHalHvspSet->gu16height[0] =(gstGlobalHalHvspSet->gu16height[0]<u16Height) ? u16Height : gstGlobalHalHvspSet->gu16height[0];
        gstGlobalHalHvspSet->gu16width[0] = (gstGlobalHalHvspSet->gu16width[0]<u16Width) ? u16Width : gstGlobalHalHvspSet->gu16width[0];
    }
    else
    {
        u32reg_22 = REG_SCL_HVSP0_22_L;
        u32reg_23 = REG_SCL_HVSP0_23_L;
        gstGlobalHalHvspSet->gu16height[0] = (gstGlobalHalHvspSet->gu16height[0]<u16Height) ? u16Height : gstGlobalHalHvspSet->gu16height[0];;
        gstGlobalHalHvspSet->gu16width[0] = (gstGlobalHalHvspSet->gu16width[0]<u16Width) ? u16Width : gstGlobalHalHvspSet->gu16width[0];
    }
    HalUtilityW2BYTEMSK(u32reg_22, u16Width,  0x3FFF);
    HalUtilityW2BYTEMSK(u32reg_23, u16Height, 0x1FFF);
    if(enID ==E_DRV_SCLHVSP_ID_1)
    {
        if(HalUtilityR2BYTE(REG_SCL_DMA0_1A_L) != u16Width)
            HalUtilityW2BYTEMSK(REG_SCL_DMA0_1A_L, u16Width, 0xFFFF);
        if(HalUtilityR2BYTE(REG_SCL_DMA0_1B_L) != u16Height)
            HalUtilityW2BYTEMSK(REG_SCL_DMA0_1B_L, u16Height,0xFFFF);
        if(HalUtilityR2BYTE(REG_SCL_DMA1_4A_L) != u16Width)
            HalUtilityW2BYTEMSK(REG_SCL_DMA1_4A_L, u16Width, 0xFFFF);
        if(HalUtilityR2BYTE(REG_SCL_DMA1_4B_L) != u16Height)
            HalUtilityW2BYTEMSK(REG_SCL_DMA1_4B_L, u16Height,0xFFFF);
        if(HalUtilityR2BYTE(REG_SCL_DMA0_4A_L) != u16Width)
            HalUtilityW2BYTEMSK(REG_SCL_DMA0_4A_L, u16Width, 0xFFFF);
        if(HalUtilityR2BYTE(REG_SCL_DMA0_4B_L) != u16Height)
            HalUtilityW2BYTEMSK(REG_SCL_DMA0_4B_L, u16Height,0xFFFF);

        if((HalUtilityR2BYTE(REG_SCL_HVSP1_20_L) != u16Width))
        {
            if(HalUtilityR2BYTE(REG_SCL_HVSP1_22_L))
            {
                //u32ratio = HAL_HVSP_RATIO(u16Width,HalUtilityR2BYTE(REG_SCL_HVSP1_22_L));
                u64Temp = (u16Width * 1048576);
                u32ratio =
                    (u32)CamOsMathDivU64(u64Temp,HalUtilityR2BYTE(REG_SCL_HVSP1_22_L),&u64Temp2);
                if(u32ratio)
                {
                    u16val_L = u32ratio & 0xFFFF;
                    u16val_H = (u32ratio >> 16) & 0x00FF;
                    u16val_H = (u16val_H|0x0100);
                    HalUtilityW2BYTEMSK(REG_SCL_HVSP1_07_L,   u16val_L, 0xFFFF);
                    HalUtilityW2BYTEMSK(REG_SCL_HVSP1_07_L+2, u16val_H, 0x01FF);
                }
                HalUtilityW2BYTEMSK(REG_SCL_HVSP1_20_L, u16Width,0xFFFF);
            }
        }
        if((HalUtilityR2BYTE(REG_SCL_HVSP1_21_L) != u16Height))
        {
            if(HalUtilityR2BYTE(REG_SCL_HVSP1_23_L))
            {
                //u32ratio = HAL_HVSP_RATIO(u16Height,HalUtilityR2BYTE(REG_SCL_HVSP1_23_L));
                u64Temp = (u16Height * 1048576);
                u32ratio =
                    (u32)CamOsMathDivU64(u64Temp,HalUtilityR2BYTE(REG_SCL_HVSP1_23_L),&u64Temp2);
                if(u32ratio)
                {
                    u16val_L = u32ratio & 0xFFFF;
                    u16val_H = (u32ratio >> 16) & 0x00FF;
                    u16val_H = (u16val_H|0x0100);
                    HalUtilityW2BYTEMSK(REG_SCL_HVSP1_09_L,   u16val_L, 0xFFFF);
                    HalUtilityW2BYTEMSK(REG_SCL_HVSP1_09_L+2, u16val_H, 0x01FF);
                }
                HalUtilityW2BYTEMSK(REG_SCL_HVSP1_21_L, u16Height,0xFFFF);
            }
        }

        //for HVSP0 to HVSP2
        if(HalSclHvspGetInputSrcMux(E_DRV_SCLHVSP_ID_3) == E_DRV_SCLHVSP_IP_MUX_HVSP)
        {
            if((HalUtilityR2BYTE(REG_SCL_HVSP2_20_L) != u16Width))
            {
                if(HalUtilityR2BYTE(REG_SCL_HVSP2_22_L))
                {
                    //u32ratio = HAL_HVSP_RATIO(u16Width,HalUtilityR2BYTE(REG_SCL_HVSP2_22_L));
                    u64Temp = (u16Width * 1048576);
                    u32ratio =
                        (u32)CamOsMathDivU64(u64Temp,HalUtilityR2BYTE(REG_SCL_HVSP2_22_L),&u64Temp2);
                    if(u32ratio)
                    {
                        u16val_L = u32ratio & 0xFFFF;
                        u16val_H = (u32ratio >> 16) & 0x00FF;
                        u16val_H = (u16val_H|0x0100);
                        HalUtilityW2BYTEMSK(REG_SCL_HVSP2_07_L,   u16val_L, 0xFFFF);
                        HalUtilityW2BYTEMSK(REG_SCL_HVSP2_07_L+2, u16val_H, 0x01FF);
                    }
                    HalUtilityW2BYTEMSK(REG_SCL_HVSP2_20_L, u16Width,0xFFFF);
                }
            }
            if((HalUtilityR2BYTE(REG_SCL_HVSP2_21_L) != u16Height))
            {
                if(HalUtilityR2BYTE(REG_SCL_HVSP2_23_L))
                {
                    //u32ratio = HAL_HVSP_RATIO(u16Height,HalUtilityR2BYTE(REG_SCL_HVSP2_23_L));
                    u64Temp = (u16Height * 1048576);
                    u32ratio =
                        (u32)CamOsMathDivU64(u64Temp,HalUtilityR2BYTE(REG_SCL_HVSP2_23_L),&u64Temp2);
                    if(u32ratio)
                    {
                        u16val_L = u32ratio & 0xFFFF;
                        u16val_H = (u32ratio >> 16) & 0x00FF;
                        u16val_H = (u16val_H|0x0100);
                        HalUtilityW2BYTEMSK(REG_SCL_HVSP2_09_L,   u16val_L, 0xFFFF);
                        HalUtilityW2BYTEMSK(REG_SCL_HVSP2_09_L+2, u16val_H, 0x01FF);
                    }
                    HalUtilityW2BYTEMSK(REG_SCL_HVSP2_21_L, u16Height,0xFFFF);
                }
            }
        }
    }
    else if(enID ==E_DRV_SCLHVSP_ID_2)
    {
        if(HalUtilityR2BYTE(REG_SCL_DMA0_62_L) != u16Width)
            HalUtilityW2BYTEMSK(REG_SCL_DMA0_62_L, u16Width, 0xFFFF);
        if(HalUtilityR2BYTE(REG_SCL_DMA0_63_L) != u16Height)
            HalUtilityW2BYTEMSK(REG_SCL_DMA0_63_L, u16Height,0xFFFF);
        if(HalUtilityR2BYTE(REG_SCL_DMA1_62_L) != u16Width)
            HalUtilityW2BYTEMSK(REG_SCL_DMA1_62_L, u16Width, 0xFFFF);
        if(HalUtilityR2BYTE(REG_SCL_DMA1_63_L) != u16Height)
            HalUtilityW2BYTEMSK(REG_SCL_DMA1_63_L, u16Height,0xFFFF);
    }
    //ToDo
    else if(enID ==E_DRV_SCLHVSP_ID_3)
    {
        if(HalUtilityR2BYTE(REG_SCL_DMA1_1A_L) != u16Width)
            HalUtilityW2BYTEMSK(REG_SCL_DMA1_1A_L, u16Width, 0xFFFF);
        if(HalUtilityR2BYTE(REG_SCL_DMA1_1B_L) != u16Height)
            HalUtilityW2BYTEMSK(REG_SCL_DMA1_1B_L, u16Height,0xFFFF);
    }
    else if(enID ==E_DRV_SCLHVSP_ID_4)
    {
        if(HalUtilityR2BYTE(REG_MDWIN1_2D_L) != u16Width)
        {
            HalUtilityW2BYTEMSK(REG_MDWIN1_2D_L, u16Width, 0xFFFF);
            HalUtilityW2BYTEMSK(REG_MDWIN1_02_L, u16Width, 0xFFFF);
        }
        if(HalUtilityR2BYTE(REG_MDWIN1_2F_L) != u16Height)
            HalUtilityW2BYTEMSK(REG_MDWIN1_2F_L, u16Height,0xFFFF);
    }
}

// input tgen
void HalSclHvspSetPatTgEn(bool bEn)
{
    HalUtilityW2BYTEMSK(REG_SCL0_40_L, bEn ? (BIT0|BIT1|BIT2|BIT15) : 0, BIT0|BIT1|BIT2|BIT15);
    HalUtilityW2BYTEMSK(REG_SCL0_70_L, bEn ? BIT0 : 0, BIT0);
    HalUtilityW2BYTEMSK(REG_SCL0_44_L, bEn ? BIT0 : 0, BIT0); // h/v
}

void HalSclHvspSetTestPatCfg(void)
{
    HalUtilityW2BYTEMSK(REG_SCL0_42_L, 0x1010, 0xFFFF); // cb h/v width
    HalUtilityW2BYTEMSK(REG_SCL0_43_L, 0x0008, 0xFFFF); // shfit time
    HalUtilityW2BYTEMSK(REG_SCL0_44_L, 0x0C20, 0xFFFF); // h/v
}


void HalSclHvspSetPatTgVsyncSt(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_71_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgVsyncEnd(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_72_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgVfdeSt(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_73_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgVfdeEnd(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_74_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgVdeSt(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_75_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgVdeEnd(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_76_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgVtt(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_77_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgHsyncSt(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_79_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgHsyncEnd(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_7A_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgHfdeSt(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_7B_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgHfdeEnd(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_7C_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgHdeSt(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_7D_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgHdeEnd(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_7E_L, u16Val, 0xFFFF);
}

void HalSclHvspSetPatTgHtt(u16 u16Val)
{
    HalUtilityW2BYTEMSK(REG_SCL0_7F_L, u16Val, 0xFFFF);
}


// LDC frame control
//#if ENABLE_HVSP_UNUSED_FUNCTION
void HalSclHvspSetRegisterForceByInst(u32 u32Reg, u8 u8Val, u8 u8Mask)
{
    HalUtilityWBYTEMSK(u32Reg, u8Val, u8Mask);
}

void HalSclHvspSetReg(u32 u32Reg, u8 u8Val, u8 u8Mask)
{
    HalUtilityWBYTEMSKDirect(u32Reg, u8Val, u8Mask);
}
u16 HalSclHvspGetCropX(void)
{
    u16 u16crop1;
    u16crop1 = HalUtilityR2BYTE(REG_SCL2_53_L);
    return u16crop1;
}

u16 HalSclHvspGetCropY(void)
{
    u16 u16crop1;
    u16crop1 = HalUtilityR2BYTE(REG_SCL2_55_L);
    return u16crop1;
}
u16 HalSclHvspGetCrop2Xinfo(void)
{
    u16 u16crop2;
    u16crop2 = HalUtilityR2BYTE(REG_SCL2_5B_L);
    return u16crop2;
}

u16 HalSclHvspGetCrop2Yinfo(void)
{
    u16 u16crop2;
    u16crop2 = HalUtilityR2BYTE(REG_SCL2_5D_L);
    return u16crop2;
}

u16 HalSclHvspGetCrop1WidthCount(void)
{
    u16 u16crop1;
    u16crop1 = HalUtilityR2BYTEDirect(REG_SCL2_70_L);
    return u16crop1;
}
u16 HalSclHvspGetCrop1HeightCount(void)
{
    u16 u16crop1;
    u16crop1 = HalUtilityR2BYTEDirect(REG_SCL2_71_L);
    return u16crop1;
}
u16 HalSclHvspGetCrop1Width(void)
{
    u16 u16crop1;
    u16crop1 = HalUtilityR2BYTE(REG_SCL2_51_L);
    return u16crop1;
}
u16 HalSclHvspGetCrop1Height(void)
{
    u16 u16crop1;
    u16crop1 = HalUtilityR2BYTE(REG_SCL2_52_L);
    return u16crop1;
}
u16 HalSclHvspGetHvspInputWidth(DrvSclHvspIdType_e enID)
{
    u16 u16size;
    if(enID == E_DRV_SCLHVSP_ID_2)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP1_20_L);
    }
    else if(enID == E_DRV_SCLHVSP_ID_3)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP2_20_L);
    }
    else if(enID == E_DRV_SCLHVSP_ID_4)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP3_20_L);
    }
    else
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP0_20_L);
    }
    return u16size;
}

u16 HalSclHvspGetHvspOutputWidth(DrvSclHvspIdType_e enID)
{
    u16 u16size;
    if(enID == E_DRV_SCLHVSP_ID_2)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP1_22_L);
    }
    else if(enID == E_DRV_SCLHVSP_ID_3)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP2_22_L);
    }
    else if(enID == E_DRV_SCLHVSP_ID_4)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP3_22_L);
    }
    else
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP0_22_L);
    }
    return u16size;
}

u16 HalSclHvspGetCrop2InputWidth(void)
{
    u16 u16size;
    u16size = HalUtilityR2BYTE(REG_SCL2_59_L);
    return u16size;
}

u16 HalSclHvspGetCrop2InputHeight(void)
{
    u16 u16size;
    u16size = HalUtilityR2BYTE(REG_SCL2_5A_L);
    return u16size;
}
u16 HalSclHvspGetCrop2OutputWidth(void)
{
    u16 u16size;
    u16size = HalUtilityR2BYTE(REG_SCL2_5C_L);
    return u16size;
}

u16 HalSclHvspGetCrop2OutputHeight(void)
{
    u16 u16size;
    u16size = HalUtilityR2BYTE(REG_SCL2_5E_L);
    return u16size;
}
u16 HalSclHvspGetCrop1En(void)
{
    u16 u16size;
    u16size = (HalUtilityR2BYTE(REG_SCL2_50_L) & BIT0);
    return u16size;
}
u16 HalSclHvspGetCrop2En(void)
{
    u16 u16size;
    u16size = (HalUtilityR2BYTE(REG_SCL2_58_L) & BIT0);
    return u16size;
}
u16 HalSclHvspGetHvspOutputHeight(DrvSclHvspIdType_e enID)
{
    u16 u16size;
    if(enID == E_DRV_SCLHVSP_ID_2)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP1_23_L);
    }
    else if(enID == E_DRV_SCLHVSP_ID_3)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP2_23_L);
    }
    else if(enID == E_DRV_SCLHVSP_ID_4)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP3_23_L);
    }
    else
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP0_23_L);
    }
    return u16size;
}
u16 HalSclHvspGetHvspInputHeight(DrvSclHvspIdType_e enID)
{
    u16 u16size;
    if(enID == E_DRV_SCLHVSP_ID_2)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP1_21_L);
    }
    else if(enID == E_DRV_SCLHVSP_ID_3)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP2_21_L);
    }
    else if(enID == E_DRV_SCLHVSP_ID_4)
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP3_21_L);
    }
    else
    {
        u16size = HalUtilityR2BYTE(REG_SCL_HVSP0_21_L);
    }
    return u16size;
}
u16 HalSclHvspGetScalingFunctionStatus(DrvSclHvspIdType_e enID)
{
    u16 u16size;
    if(enID == E_DRV_SCLHVSP_ID_2)
    {
        u16size = (HalUtilityR2BYTE(REG_SCL_HVSP1_08_L) & BIT8)>>8;
        u16size |= (HalUtilityR2BYTE(REG_SCL_HVSP1_0A_L)& BIT8)>>7;
    }
    else if(enID == E_DRV_SCLHVSP_ID_3)
    {
        u16size = (HalUtilityR2BYTE(REG_SCL_HVSP2_08_L) & BIT8)>>8;
        u16size |= (HalUtilityR2BYTE(REG_SCL_HVSP2_0A_L)& BIT8)>>7;
    }
    else if(enID == E_DRV_SCLHVSP_ID_4)
    {
        u16size = (HalUtilityR2BYTE(REG_SCL_HVSP3_08_L) & BIT8)>>8;
        u16size |= (HalUtilityR2BYTE(REG_SCL_HVSP3_0A_L)& BIT8)>>7;
    }
    else
    {
        u16size = (HalUtilityR2BYTE(REG_SCL_HVSP0_08_L) & BIT8)>>8;
        u16size |= (HalUtilityR2BYTE(REG_SCL_HVSP0_0A_L)& BIT8)>>7;
    }
    return u16size;
}
void HalSclHvspWriteReg(u32 u32Reg,u16 u16Val,u16 u16Mask)
{
    HalUtilityW2BYTEMSKDirectCmdq(u32Reg, u16Val, u16Mask);//sec
}
void HalSclHvspSRAMDumpHSP(u32 u32Sram,u32 u32reg)
{
    u16 u16addr = 0,u16tvalue = 0,u16tcount = 0;
    //clear
    for(u16addr = 64;u16addr<128;u16addr++)
    {
        HalSclHvspWriteReg(u32reg, BIT0, BIT0);//Yenable
        HalSclHvspWriteReg(u32reg+2, u16addr, 0xFF);
        HalSclHvspWriteReg(u32reg+4, 0, 0xFFFF);
        HalSclHvspWriteReg(u32reg+6, 0, 0xFFFF);
        HalSclHvspWriteReg(u32reg+8, 0, 0xFF);
        HalSclHvspWriteReg(u32reg, BIT0|BIT8, BIT0|BIT8);//W pulse
        HalSclHvspWriteReg(u32reg, 0, BIT0);//Yenable
    }
    //SRAM 0 //0~64 entry is V  65~127 is H

    for(u16addr=0;u16addr<(PQ_IP_SRAM1_SIZE_Main/5);u16addr++)
    {
           // 64~127
        HalSclHvspWriteReg(u32reg, BIT0, BIT0);//Yenable
        HalSclHvspWriteReg(u32reg+2, u16addr+64, 0xFF);
        if(u32Sram&0x1)
        {
            u16tvalue = MST_SRAM1_Main[((u32Sram&0xF0)>>4)][u16tcount] |
                (MST_SRAM1_Main[((u32Sram&0xF0)>>4)][u16tcount+1]<<8);
        }
        else
        {
            u16tvalue = MST_SRAM2_Main[((u32Sram&0xF0)>>4)][u16tcount] |
                (MST_SRAM2_Main[((u32Sram&0xF0)>>4)][u16tcount+1]<<8);
        }
        SCL_DBG(SCL_DBG_LV_DRVHVSP()&EN_DBGMG_HVSPLEVEL_ELSE, "[HVSP]VYtval12:%hx\n", u16tvalue);
        HalSclHvspWriteReg(u32reg+4, u16tvalue, 0xFFFF);
        if(u32Sram&0x1)
        {
            u16tvalue = MST_SRAM1_Main[((u32Sram&0xF0)>>4)][u16tcount+2] |
                (MST_SRAM1_Main[((u32Sram&0xF0)>>4)][u16tcount+3]<<8);
        }
        else
        {
            u16tvalue = MST_SRAM2_Main[((u32Sram&0xF0)>>4)][u16tcount+2] |
                (MST_SRAM2_Main[((u32Sram&0xF0)>>4)][u16tcount+3]<<8);
        }
        HalSclHvspWriteReg(u32reg+6, u16tvalue, 0xFFFF);
        SCL_DBG(SCL_DBG_LV_DRVHVSP()&EN_DBGMG_HVSPLEVEL_ELSE, "[HVSP]VYtval34:%hx\n", u16tvalue);
        if(u32Sram&0x1)
        {
            u16tvalue = MST_SRAM1_Main[((u32Sram&0xF0)>>4)][u16tcount+4];
        }
        else
        {
            u16tvalue = MST_SRAM2_Main[((u32Sram&0xF0)>>4)][u16tcount+4];
        }
        SCL_DBG(SCL_DBG_LV_DRVHVSP()&EN_DBGMG_HVSPLEVEL_ELSE, "[HVSP]Vtval5:%hx\n", u16tvalue);
        HalSclHvspWriteReg(u32reg+8, u16tvalue, 0xFF);
        HalSclHvspWriteReg(u32reg, BIT0|BIT8|BIT9, BIT0|BIT8|BIT9);//W pulse
        HalSclHvspWriteReg(u32reg, 0, BIT0);//Yenable
        u16tcount += 5;
    }
    //SRAM 1 //128~256 entry  link to 0~127 , so do not use
    u16tcount = 0;
}
void HalSclHvspSRAMDumpVSP(u32 u32Sram,u32 u32reg)
{
    u16 u16addr = 0,u16tvalue = 0,u16tcount = 0;
    //clear
    for(u16addr = 0;u16addr<64;u16addr++)
    {
        HalSclHvspWriteReg(u32reg, BIT0, BIT0);//Yenable
        HalSclHvspWriteReg(u32reg+2, u16addr, 0xFF);
        HalSclHvspWriteReg(u32reg+4, 0, 0xFFFF);
        HalSclHvspWriteReg(u32reg+6, 0, 0xFFFF);
        HalSclHvspWriteReg(u32reg+8, 0, 0xFF);
        HalSclHvspWriteReg(u32reg, BIT8|BIT0, BIT8|BIT0);//W pulse
        HalSclHvspWriteReg(u32reg, 0, BIT0);//Yenable
    }
    //SRAM 0 ///0~64 entry is V  65~127 is H

    for(u16addr=0;u16addr<(PQ_IP_SRAM1_SIZE_Main/5);u16addr++)
    {
        HalSclHvspWriteReg(u32reg, BIT0, BIT0);//Yenable
        HalSclHvspWriteReg(u32reg+2, u16addr, 0xFF);
        if(u32Sram&0x1)
        {
            u16tvalue = MST_SRAM1_Main[((u32Sram&0xF0)>>4)][u16tcount] |
                (MST_SRAM1_Main[((u32Sram&0xF0)>>4)][u16tcount+1]<<8);
        }
        else
        {
            u16tvalue = MST_SRAM2_Main[((u32Sram&0xF0)>>4)][u16tcount] |
                (MST_SRAM2_Main[((u32Sram&0xF0)>>4)][u16tcount+1]<<8);
        }
        HalSclHvspWriteReg(u32reg+4, u16tvalue, 0xFFFF);
        SCL_DBG(SCL_DBG_LV_DRVHVSP()&EN_DBGMG_HVSPLEVEL_ELSE, "[HVSP]Ytval12:%hx\n", u16tvalue);
        if(u32Sram&0x1)
        {
            u16tvalue = MST_SRAM1_Main[((u32Sram&0xF0)>>4)][u16tcount+2] |
                (MST_SRAM1_Main[((u32Sram&0xF0)>>4)][u16tcount+3]<<8);
        }
        else
        {
            u16tvalue = MST_SRAM2_Main[((u32Sram&0xF0)>>4)][u16tcount+2] |
                (MST_SRAM2_Main[((u32Sram&0xF0)>>4)][u16tcount+3]<<8);
        }
        HalSclHvspWriteReg(u32reg+6, u16tvalue, 0xFFFF);
        SCL_DBG(SCL_DBG_LV_DRVHVSP()&EN_DBGMG_HVSPLEVEL_ELSE, "[HVSP]Ytval34:%hx\n", u16tvalue);
        if(u32Sram&0x1)
        {
            u16tvalue = MST_SRAM1_Main[((u32Sram&0xF0)>>4)][u16tcount+4];
        }
        else
        {
            u16tvalue = MST_SRAM2_Main[((u32Sram&0xF0)>>4)][u16tcount+4];
        }
        HalSclHvspWriteReg(u32reg+8, u16tvalue, 0xFF);
        SCL_DBG(SCL_DBG_LV_DRVHVSP()&EN_DBGMG_HVSPLEVEL_ELSE, "[HVSP]Ytval5:%hx\n", u16tvalue);
        HalSclHvspWriteReg(u32reg, BIT0|BIT8|BIT9, BIT0|BIT8|BIT9);//W pulse
        HalSclHvspWriteReg(u32reg, 0, BIT0);//Yenable
        u16tcount += 5;
    }
    //SRAM 1 //128~256 entry  link to 0~127 , so do not use
    u16tcount = 0;
}
bool HalSclHvspSetSRAMDump(HalSclHvspSramDumpType_e endump,u32 u32Sram,u32 u32reg)
{
    if(endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V || endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_1||
        endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_2|| endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_3)
    {
        HalSclHvspSRAMDumpVSP(u32Sram,u32reg);
    }
    else if(endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H || endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_1||
        endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_2|| endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_3)
    {
        HalSclHvspSRAMDumpHSP(u32Sram,u32reg);
    }
    return 1;
}
bool HalSclHvspSramDump(HalSclHvspSramDumpType_e endump,u32 u32Sram)
{
    u16 u16clkreg;
    bool bRet;
    u32 u32reg = endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V ? REG_SCL_HVSP0_41_L:
                    endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_1 ? REG_SCL_HVSP1_41_L:
                    endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_2 ? REG_SCL_HVSP2_41_L:
                    endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_3 ? REG_SCL_HVSP3_41_L:
                    endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H ? REG_SCL_HVSP0_41_L:
                    endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_1 ? REG_SCL_HVSP1_41_L:
                    endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_2 ? REG_SCL_HVSP2_41_L:
                    endump == E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_3 ? REG_SCL_HVSP3_41_L:
                        0;
    //clk open
    u16clkreg = HalUtilityR2BYTEDirect(REG_SCL_CLK_51_L);
    HalUtilityW2BYTEMSK(REG_SCL_CLK_51_L,0x0,0xFFFF);
    bRet = HalSclHvspSetSRAMDump(endump,u32Sram,u32reg);

    //clk close
    HalUtilityW2BYTEMSK(REG_SCL_CLK_51_L,u16clkreg,0xFFFF);
    return bRet;
}
void HalSclHvspVtrackSetUUID(void)
{
    u64 u64UUid = 0;
    HalUtilityW2BYTEMSK(REG_SCL1_29_L, (u16)(u64UUid&0xFFFF), 0xFFFF);
    HalUtilityW2BYTEMSK(REG_SCL1_2A_L, (u16)((u64UUid>>16)&0xFFFF),0xFFFF);
}

#define  VTRACK_KEY_SETTING_LENGTH 8

static u8 u8VtrackKey[VTRACK_KEY_SETTING_LENGTH]=
{
    0xaa,0x13,0x46,0x90,0x28,0x35,0x29,0xFE,
};

void HalSclHvspVtrackSetKey(bool bUserDefinded, u8 *pu8Setting)
{
    /*
     * Default Setting:
     * setting1 [0e]              => 8'h00
     * setting2 [0f]              => 8'h00
     * setting3 [1a:10]           => 165'h04 21 08 418c6318c4 21084210842108421086318c53
     */

    u8 *pu8Data = NULL;
    u16 u16Index = 0;
    u16 u16Data = 0;

    if (bUserDefinded == TRUE)
    {
        pu8Data = pu8Setting;
    }
    else
    {
        pu8Data = &u8VtrackKey[0];
    }
    for (u16Index = 0; u16Index < VTRACK_KEY_SETTING_LENGTH; u16Index = u16Index+2)
    {
        u8 u8Offset = (( (u16Index) / 2) *2 );
        u32 u32Addr = REG_SCL1_24_L + u8Offset;

        if ( (VTRACK_KEY_SETTING_LENGTH - u16Index) >= 2)
        {
            u16Data = (u16)pu8Data[u16Index +1];
            u16Data = (u16Data << 8) + (u16) pu8Data[u16Index];
            HalUtilityW2BYTEMSK(u32Addr, u16Data, 0xffff);
        }
    }

}

void HalSclHvspVtrackSetPayloadData(u16 u16Timecode, u8 u8OperatorID)
{
    /*
     * reg_payload_use from 0x28 to 0x2C (1B - 1F)
     * {14'h0, TimeCode[55:40], Unique ID (OTP)[39:8], Operator ID[7:0]}
     */
    HalUtilityW2BYTEMSK(REG_SCL1_28_L, (MS_U16)u16Timecode, 0xFFFF);
    HalUtilityW2BYTEMSK(REG_SCL1_2B_L, (MS_U16)u8OperatorID , 0x00ff);
}

#define  VTRACK_SETTING_LENGTH 23

static u8 u8VtrackSetting[VTRACK_SETTING_LENGTH]=
{
    0x00, 0x00, 0xe8, 0x18, 0x32,
    0x86, 0x10, 0x42, 0x08, 0x21,
    0x84, 0x10, 0x42, 0x08, 0x21,
    0xc4, 0x18, 0x63, 0x8c, 0x41,
    0x08, 0x21, 0x04,
};

void HalSclHvspVtrackSetUserDefindedSetting(bool bUserDefinded, u8 *pu8Setting)
{
    /*
     * Default Setting:
     * setting1 [0e]              => 8'h00
     * setting2 [0f]              => 8'h00
     * setting3 [1a:10]           => 165'h04 21 08 418c6318c4 21084210842108421086318c53
     */

    u8 *pu8Data = NULL;
    u16 u16Index = 0;
    u16 u16Data = 0;

    if (bUserDefinded == TRUE)
    {
        pu8Data = pu8Setting;
    }
    else
    {
        pu8Data = &u8VtrackSetting[0];
    }
    //MenuLoad enable
    //HalUtilityW2BYTEMSK(REG_SCL1_2E_L, pu8Data[0], 0xff);//for dummy use it
    //HalUtilityW2BYTEMSK(REG_SCL1_2F_L, pu8Data[1], 0xff);  //for dummy use it
    for (u16Index = 2; u16Index < VTRACK_SETTING_LENGTH; u16Index = u16Index+2)
    {
        u8 u8Offset = (( (u16Index - 2) / 2) *2 );
        u32 u32Addr = REG_SCL1_30_L + u8Offset;

        if ( (VTRACK_SETTING_LENGTH - u16Index) >= 2)
        {
            u16Data = (u16)pu8Data[u16Index +1];
            u16Data = (u16Data << 8) + (u16) pu8Data[u16Index];
            HalUtilityW2BYTEMSK(u32Addr, u16Data, 0xffff);
        }
        else
        {
            u16Data = (u16) pu8Data[u16Index];
            HalUtilityW2BYTEMSK(u32Addr, u16Data, 0x00ff);
        }
    }

}

#define VIP_VTRACK_MODE 0x1c   //[3]:v_sync_inv_en ;[2]:h_sync_inv_en [4]mux
void HalSclHvspVtrackEnable(u8 u8FrameRate, HalSclHvspVtrackEnableType_e bEnable)
{
    //FrameRateIn     => 8'h1E
    HalUtilityW2BYTEMSK(REG_SCL1_21_L , u8FrameRate, 0xFF);

    //EnableIn              => 1
    //DebugEn               => 0
    if (bEnable == E_HAL_SCLVIP_VTRACK_ENABLE_ON)
    {
        HalUtilityW2BYTEMSK(REG_SCL1_20_L , BIT0, BIT1|BIT0|BIT2 );
        HalUtilityW2BYTEMSK(REG_SCL1_22_L , VIP_VTRACK_MODE, 0x3F );
    }
    else if(bEnable == E_HAL_SCLVIP_VTRACK_ENABLE_DEBUG)
    {
        HalUtilityW2BYTEMSK(REG_SCL1_20_L , BIT1|BIT0, BIT1|BIT0|BIT2 );
        HalUtilityW2BYTEMSK(REG_SCL1_22_L , VIP_VTRACK_MODE, 0x3F );
    }
    else
    {
        HalUtilityW2BYTEMSK(REG_SCL1_20_L , 0x00, BIT1|BIT0|BIT2 );
        HalUtilityW2BYTEMSK(REG_SCL1_22_L , VIP_VTRACK_MODE, 0x3F );
    }
}

#undef HAL_HVSP_C
