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
#define HAL_VIP_C


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_utility.h"
// Internal Definition
#include "hal_scl_reg.h"
#include "drv_scl_vip.h"
#include "hal_scl_util.h"
#include "hal_scl_vip.h"
#include "drv_scl_pq_define.h"
#include "Infinity_Iq.h"             // table config parameter
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HAL_SCLVIP_DBG(x)
#define HAL_SCLVIP_MUTEX_LOCK()            //DrvSclOsObtainMutex(_HalVIP_Mutex,SCLOS_WAIT_FOREVER)
#define HAL_SCLVIP_MUTEX_UNLOCK()          //DrvSclOsReleaseMutex(_HalVIP_Mutex)
#define WDR_SRAM_NUM 8
#define WDR_SRAM_BYTENUM 0
#define WDR_SRAM_USERBYTENUM (81)
#define GAMMAY_SRAM_BYTENUM (PQ_IP_YUV_Gamma_tblY_SRAM_SIZE_Main/4)
#define GAMMAU_SRAM_BYTENUM (PQ_IP_YUV_Gamma_tblU_SRAM_SIZE_Main/4)
#define GAMMAV_SRAM_BYTENUM (PQ_IP_YUV_Gamma_tblV_SRAM_SIZE_Main/4)
#define GAMMA10to12R_SRAM_BYTENUM (PQ_IP_ColorEng_GM10to12_Tbl_R_SRAM_SIZE_Main/4)
#define GAMMA10to12G_SRAM_BYTENUM (PQ_IP_ColorEng_GM10to12_Tbl_G_SRAM_SIZE_Main/4)
#define GAMMA10to12B_SRAM_BYTENUM (PQ_IP_ColorEng_GM10to12_Tbl_B_SRAM_SIZE_Main/4)
#define GAMMA12to10R_SRAM_BYTENUM (PQ_IP_ColorEng_GM12to10_CrcTbl_R_SRAM_SIZE_Main/4)
#define GAMMA12to10G_SRAM_BYTENUM (PQ_IP_ColorEng_GM12to10_CrcTbl_G_SRAM_SIZE_Main/4)
#define GAMMA12to10B_SRAM_BYTENUM (PQ_IP_ColorEng_GM12to10_CrcTbl_B_SRAM_SIZE_Main/4)
#define NRBUFFERCNT 8
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
//keep
u32 VIP_RIU_BASE = 0;
s32 _HalVIP_Mutex = -1;

//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------
void HalSclVipExit(void)
{
    if(_HalVIP_Mutex != -1)
    {
        DrvSclOsDeleteMutex(_HalVIP_Mutex);
        _HalVIP_Mutex = -1;
    }
}

void HalSclVipSetRiuBase(u32 u32riubase)
{
    char word[] = {"_HalVIP_Mutex"};
    VIP_RIU_BASE = u32riubase;
    _HalVIP_Mutex = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, word, SCLOS_PROCESS_SHARED);

    if (_HalVIP_Mutex == -1)
    {
        SCL_ERR("[DRVHVSP]%s(%d): create mutex fail\n", __FUNCTION__, __LINE__);
    }
}
void _HalSclVipSramDumpIhcIcc(HalSclVipSramDumpType_e endump,u32 u32reg)
{
    u8  u8sec = 0;
    HalSclVipSramSecNum_e enSecNum;
    u16 u16addr = 0,u16tvalue = 0,u16tcount = 0,u16readdata;
    HalUtilityW2BYTEMSK(u32reg, BIT0, BIT0);//IOenable
    for(u8sec=0;u8sec<4;u8sec++)
    {
        switch(u8sec)
        {
            case 0:
                enSecNum = E_HAL_SCLVIP_SRAM_SEC_0;
                break;
            case 1:
                enSecNum = E_HAL_SCLVIP_SRAM_SEC_1;
                break;
            case 2:
                enSecNum = E_HAL_SCLVIP_SRAM_SEC_2;
                break;
            case 3:
                enSecNum = E_HAL_SCLVIP_SRAM_SEC_3;
                break;
            default:
                break;

        }
        HalUtilityW2BYTEMSK(u32reg, u8sec<<1, BIT1|BIT2);//sec
        for(u16addr=0;u16addr<enSecNum;u16addr++)
        {
            if(endump == E_HAL_SCLVIP_SRAM_DUMP_IHC)
            {
                u16tvalue = MST_VIP_IHC_CRD_SRAM_Main[0][u16tcount] | (MST_VIP_IHC_CRD_SRAM_Main[0][u16tcount+1]<<8);
            }
            else
            {
                u16tvalue = MST_VIP_ICC_CRD_SRAM_Main[0][u16tcount] | (MST_VIP_ICC_CRD_SRAM_Main[0][u16tcount+1]<<8);
            }
            HalUtilityW2BYTEMSK(u32reg+2, u16addr, 0x01FF);//addr
            HalUtilityW2BYTEMSK(u32reg+4, (u16)u16tvalue, 0x3FF);//data
            HalUtilityW2BYTEMSK(u32reg+4, BIT15, BIT15);//wen
            //if(SCL_DBG_LV_DRVVIP()&EN_DBGMG_VIPLEVEL_VIPLOG)
            {
                HalUtilityW2BYTEMSK(u32reg+2, BIT15, BIT15);//ren
                SCL_DBG(SCL_DBG_LV_DRVVIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[ICCIHC]tval:%hx\n", u16tvalue);
                u16readdata=HalUtilityR2BYTEDirect(u32reg+6);
                SCL_DBG(SCL_DBG_LV_DRVVIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[ICCIHC]reg tval:%hx\n", u16readdata);
            }
            u16tcount+=2;
        }
    }

    HalUtilityW2BYTEMSK(u32reg, 0, BIT0);//IOenable
}
bool _HalSclVipSetSramDump(HalSclVipSramDumpType_e endump,u32 u32Sram,u32 u32reg)
{
    if(endump == E_HAL_SCLVIP_SRAM_DUMP_IHC || endump == E_HAL_SCLVIP_SRAM_DUMP_ICC)
    {
        _HalSclVipSramDumpIhcIcc(endump,u32reg);
    }
    return 1;
}
bool HalSclVipSetWdrTbl(HalSclVipWdrTblType_e enWdrType,void *pTbl)
{
    bool bRet = 1;
    u16 u16idx;
    u16 u16cnt;
    u16 u16Tvalue;
    u16 *p16Tbllocal;
    u32 u32reg = enWdrType == E_HAL_SCLVIP_WDR_TBL_NL ? REG_VIP_WDR_40_L :
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_SAT ?  REG_VIP_WDR_51_L :
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_FMAP ? REG_VIP_WDR1_40_L:
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_ALPHA? REG_VIP_WDR1_51_L:
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_L ? REG_VIP_WDR1_62_L:
                        0;
    if(pTbl==NULL)
    {
        p16Tbllocal = enWdrType == E_HAL_SCLVIP_WDR_TBL_NL ? MST_ColorEng_WDR_nonlinear_c_SRAM_Main :
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_SAT ?  MST_ColorEng_WDR_sat_c_SRAM_Main :
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_FMAP ? MST_ColorEng_WDR_f_c_SRAM_Main :
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_ALPHA? MST_ColorEng_WDR_alpha_c_SRAM_Main :
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_L ? MST_ColorEng_WDR_linear_c_SRAM_Main:
                        pTbl;
    }
    else
    {
        p16Tbllocal = enWdrType == E_HAL_SCLVIP_WDR_TBL_NL ? pTbl :
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_SAT ?  pTbl+SCLVIP_WDR_TBL_NL_SIZE :
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_FMAP ? pTbl+SCLVIP_WDR_TBL_NL_SIZE+SCLVIP_WDR_TBL_SAT_SIZE :
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_ALPHA? pTbl+SCLVIP_WDR_TBL_NL_SIZE+SCLVIP_WDR_TBL_SAT_SIZE
                        +SCLVIP_WDR_TBL_FMAP_SIZE :
                    enWdrType == E_HAL_SCLVIP_WDR_TBL_L ? pTbl+SCLVIP_WDR_TBL_NL_SIZE+SCLVIP_WDR_TBL_SAT_SIZE
                        +SCLVIP_WDR_TBL_FMAP_SIZE +SCLVIP_WDR_TBL_ALPHA_SIZE:
                        pTbl;
    }
    if(enWdrType== E_HAL_SCLVIP_WDR_TBL_SAT)
    {
        for(u16idx = 0;u16idx<SCLVIP_WDR_TBL_T;u16idx++)
        {
            if(u16idx<(SCLVIP_WDR_TBL_T-1))
            {
                for(u16cnt = 0;u16cnt<(SCLVIP_WDR_TBL_CNT/2);u16cnt++)
                {
                    u16Tvalue = *(p16Tbllocal+u16cnt+(u16idx*(SCLVIP_WDR_TBL_CNT/2)));
                    HalUtilityW2BYTETbl(u32reg+2+u16cnt*2,u16Tvalue,0xFFFF);
                }
            }
            else
            {
                u16cnt = 0;
                u16Tvalue = (u8)*(p16Tbllocal+u16cnt+(u16idx*(SCLVIP_WDR_TBL_CNT/2)));
                HalUtilityW2BYTETbl(u32reg+2+u16cnt*2,u16Tvalue,0x00FF);
            }
            HalUtilityW2BYTETbl(u32reg,(0x1100+u16idx),0xFFFF);
        }
    }
    else
    {
        for(u16idx = 0;u16idx<SCLVIP_WDR_TBL_T;u16idx++)
        {
            if(u16idx<(SCLVIP_WDR_TBL_T-1))
            {
                for(u16cnt = 0;u16cnt<(SCLVIP_WDR_TBL_CNT);u16cnt++)
                {
                    u16Tvalue = *(p16Tbllocal+u16cnt+(u16idx*SCLVIP_WDR_TBL_CNT));
                    HalUtilityW2BYTETbl(u32reg+2+u16cnt*2,u16Tvalue,0xFFFF);
                }
            }
            else
            {
                u16cnt = 0;
                u16Tvalue = *(p16Tbllocal+u16cnt+(u16idx*SCLVIP_WDR_TBL_CNT));
                HalUtilityW2BYTETbl(u32reg+2+u16cnt*2,u16Tvalue,0xFFFF);
            }
            HalUtilityW2BYTETbl(u32reg,(0x1100+u16idx),0xFFFF);
        }
    }
    return bRet;
}
bool HalSclVipSramDump(HalSclVipSramDumpType_e endump,u32 u32Sram)
{
    u16 u16clkreg;
    bool bRet;
    u32 u32reg = endump == E_HAL_SCLVIP_SRAM_DUMP_IHC ? REG_VIP_ACE2_7C_L :
                    endump == E_HAL_SCLVIP_SRAM_DUMP_ICC ?  REG_VIP_ACE2_78_L :
                        0;
    //clk open
    u16clkreg = HalUtilityR2BYTEDirect(REG_SCL_CLK_51_L);
    HalUtilityW2BYTEMSKDirect(REG_SCL_CLK_51_L,0x0,0x1F);
    bRet = _HalSclVipSetSramDump(endump,u32Sram,u32reg);

    //clk close
    HalUtilityW2BYTEMSKDirect(REG_SCL_CLK_51_L,u16clkreg,0x1F);
    return bRet;
}

//-----------------------DLC
void HalSclVipDlcHistVarOnOff(u16 u16var)
{
    HalUtilityW2BYTEMSK(REG_VIP_DLC_04_L,u16var,0x25a2);
}

void HalSclVipSetDlcstatMIU(u8 u8value,u32 u32addr1,u32 u32addr2)
{
    HalUtilityW2BYTEMSK(REG_VIP_MWE_15_L,u8value,0x0001);
    HalUtilityW4BYTE(REG_VIP_MWE_18_L,u32addr1>>4);
    HalUtilityW4BYTE(REG_VIP_MWE_1A_L,u32addr2>>4);
}
#if defined(SCLOS_TYPE_LINUX_DEBUG)
bool HalSclVipGetYeeOnOff(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_VIP_YEE_25_L)&BIT2)? 1: 0;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_YEE_25_L)&BIT1)? 1: 0)<<1;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_YEE_72_L)&BIT0)? 1: 0)<<2;
    return bRet;
}
bool HalSclVipGetMxnrOnOff(void)
{
    return HalUtilityR2BYTE(REG_VIP_SCNR_00_L)&BIT0;
}
bool HalSclVipGetUvadjOnOff(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_VIP_SCNR_20_L)&BIT0)? 1: 0;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_SCNR_20_L)&BIT1)? 1: 0)<<1;
    return bRet;
}
bool HalSclVipGetXnrOnOff(void)
{
    return HalUtilityR2BYTE(REG_VIP_SCNR_50_L)&BIT0;
}
bool HalSclVipGetPfcOnOff(void)
{
    return HalUtilityR2BYTE(REG_VIP_WDR_00_L)&BIT0;
}
bool HalSclVipGetYcuvmOnOff(void)
{
    return (HalUtilityR2BYTE(REG_VIP_SCNR_60_L)&BIT0)? 0: 1;
}
bool HalSclVipGetCTOnOff(void)
{
    return HalUtilityR2BYTE(REG_VIP_SCNR_68_L)&BIT0;
}
bool HalSclVipGetYUVGammaOnOff(void)
{
    return HalUtilityR2BYTE(REG_VIP_SCNR_40_L)&BIT0;
}
bool HalSclVipGetY2ROnOff(void)
{
    return HalUtilityR2BYTE(REG_SCL_HVSP2_60_L)&BIT0;
}
bool HalSclVipGetR2YOnOff(void)
{
    return HalUtilityR2BYTE(REG_SCL_HVSP1_6C_L)&BIT0;
}
bool HalSclVipGetGMA2COnOff(void)
{
    return HalUtilityR2BYTE(REG_SCL_HVSP1_76_L)&BIT0;
}
bool HalSclVipGetGMC2AOnOff(void)
{
    return HalUtilityR2BYTE(REG_SCL_HVSP1_77_L)&BIT0;
}
bool HalSclVipGetHsvOnOff(void)
{
    return HalUtilityR2BYTE(REG_SCL_HVSP0_60_L)&BIT0;
}
bool HalSclVipGetCcmOnOff(void)
{
    return HalUtilityR2BYTE(REG_SCL_HVSP1_62_L)&BIT0;
}
#endif
bool HalSclVipGetWdrOnOff(void)
{
    return HalUtilityR2BYTE(REG_VIP_WDR1_00_L)&BIT0;
}
void HalSclVipHwReset(void)
{
    HalUtilityW2BYTEMSKDirect(REG_VIP_MWE_15_L,0,0x0001);
    HalUtilityW2BYTEMSKDirect(REG_SCL_ARBSHP_00_L,0,BIT0);
    HalSclVipInitY2R();
}
void HalSclVipSetRoiHistSrc(DrvSclVipWdrRoiSrcType_e enType)
{
    HalUtilityW2BYTEMSK(REG_SCL_ARBSHP_00_L,(enType==E_DRV_SCLVIP_ROISRC_BEFORE_WDR) ? 0 : BIT1,BIT1);
}
void _HalSclVipSetRoiHistWinCfg(DrvSclVipWdrRoiConfig_t *stRoiCfg,u8 u8Win)
{
    u16 u16EnSkip;
    u32 u32Reg;
    u16EnSkip = (u8Win==0) ? BIT2 : (u8Win==1) ? BIT3 : (u8Win==2) ? BIT4 : BIT5;
    u32Reg = (u8Win==0) ? REG_SCL_ARBSHP_01_L : (u8Win==1) ? REG_SCL_ARBSHP_13_L :
    (u8Win==2) ? REG_SCL_ARBSHP_25_L : REG_SCL_ARBSHP_37_L;
    HalUtilityW2BYTEMSK(REG_SCL_ARBSHP_00_L,(stRoiCfg->bEnSkip) ? u16EnSkip : 0,u16EnSkip);
    //xy

    HalUtilityW2BYTEMSK(u32Reg,(stRoiCfg->u16RoiAccX[0]),0x1FFF);
    HalUtilityW2BYTEMSK(u32Reg+2,(stRoiCfg->u16RoiAccY[0]),0x1FFF);
    HalUtilityW2BYTEMSK(u32Reg+4,(stRoiCfg->u16RoiAccX[1]),0x1FFF);
    HalUtilityW2BYTEMSK(u32Reg+6,(stRoiCfg->u16RoiAccY[1]),0x1FFF);
    HalUtilityW2BYTEMSK(u32Reg+8,(stRoiCfg->u16RoiAccX[2]),0x1FFF);
    HalUtilityW2BYTEMSK(u32Reg+10,(stRoiCfg->u16RoiAccY[2]),0x1FFF);
    HalUtilityW2BYTEMSK(u32Reg+12,(stRoiCfg->u16RoiAccX[3]),0x1FFF);
    HalUtilityW2BYTEMSK(u32Reg+14,(stRoiCfg->u16RoiAccY[3]),0x1FFF);
}
void HalSclVipGetRoiHistCfg(u16 idx, DrvSclVipWdrRoiConfig_t *stRoiCfg)
{
    u32 u32Reg = 0;
    switch(idx)
    {
        case 0:
            u32Reg = REG_SCL_ARBSHP_01_L;
            break;
        case 1:
            u32Reg = REG_SCL_ARBSHP_13_L;
            break;
        case 2:
            u32Reg = REG_SCL_ARBSHP_25_L;
            break;
        case 3:
            u32Reg = REG_SCL_ARBSHP_37_L;
            break;
        default:
            SCL_ERR("[HALSCLVIP]%s idx not support\n",__FUNCTION__);
            break;
    }
    (stRoiCfg->u16RoiAccX[0]) = HalUtilityR2BYTE(u32Reg);
    (stRoiCfg->u16RoiAccY[0]) = HalUtilityR2BYTE(u32Reg+2);
    (stRoiCfg->u16RoiAccX[1]) = HalUtilityR2BYTE(u32Reg+4);
    (stRoiCfg->u16RoiAccY[1]) = HalUtilityR2BYTE(u32Reg+6);
    (stRoiCfg->u16RoiAccX[2]) = HalUtilityR2BYTE(u32Reg+8);
    (stRoiCfg->u16RoiAccY[2]) = HalUtilityR2BYTE(u32Reg+10);
    (stRoiCfg->u16RoiAccX[3]) = HalUtilityR2BYTE(u32Reg+12);
    (stRoiCfg->u16RoiAccY[3]) = HalUtilityR2BYTE(u32Reg+14);
}
void HalSclVipSetRoiHistCfg(u16 idx, DrvSclVipWdrRoiConfig_t *stRoiCfg)
{
    switch(idx)
    {
        case 0:
            _HalSclVipSetRoiHistWinCfg(stRoiCfg,0);
            break;
        case 1:
            _HalSclVipSetRoiHistWinCfg(stRoiCfg,1);
            break;
        case 2:
            _HalSclVipSetRoiHistWinCfg(stRoiCfg,2);
            break;
        case 3:
            _HalSclVipSetRoiHistWinCfg(stRoiCfg,3);
            break;
        default:
            SCL_ERR("[HALSCLVIP]%s idx not support\n",__FUNCTION__);
            break;
    }
}
void HalSclVipSetRoiHistBaseAddr(u16 idx, u32 u32BaseAddr)
{
    u32 u32Addr;
    u32 Reg = 0;
    u32Addr = ((u32BaseAddr)>>5);//256bit MIU
    switch(idx)
    {
        case 0://2048byte
            Reg = REG_SCL_ARBSHP_49_L;
            break;
        case 1://256byte
            Reg = REG_SCL_ARBSHP_4B_L;
            break;
        case 2://256byte
            Reg = REG_SCL_ARBSHP_4D_L;
            break;
        case 3://256byte
            Reg = REG_SCL_ARBSHP_4F_L;
            break;
        default:
            SCL_ERR("[HALSCLVIP]%s idx not support\n",__FUNCTION__);
            SCLOS_BUG();
            break;
    }
    HalUtilityW2BYTEMSK(Reg,u32Addr,0xFFFF);
    HalUtilityW2BYTEMSK(Reg+2,(u32Addr>>16),0x7FF);
}
void HalSclVipReSetRoiHistCfg(u16 idx)
{
    DrvSclVipWdrRoiConfig_t stRoiCfg;
    DrvSclOsMemset(&stRoiCfg,0,sizeof(DrvSclVipWdrRoiConfig_t));
    switch(idx)
    {
        case 0:
            _HalSclVipSetRoiHistWinCfg(&stRoiCfg,0);
            break;
        case 1:
            _HalSclVipSetRoiHistWinCfg(&stRoiCfg,1);
            break;
        case 2:
            _HalSclVipSetRoiHistWinCfg(&stRoiCfg,2);
            break;
        case 3:
            _HalSclVipSetRoiHistWinCfg(&stRoiCfg,3);
            break;
        default:
            SCL_ERR("[HALSCLVIP]%s idx not support\n",__FUNCTION__);
            break;
    }
}
void HalSclVipSetRoiHistOnOff(bool bEn)
{
    HalUtilityW2BYTEMSK(REG_SCL_ARBSHP_00_L,bEn ? BIT0 : 0,BIT0);
}
void HalSclVipSetWdrMultiSensor(bool bEn)
{
    HalUtilityW2BYTEMSK(REG_VIP_WDR1_00_L,bEn ? BIT4 : 0,BIT4);
}
void HalSclVipSetWdrMloadBuffer(u32 u32Buffer)
{
    u32 u32Addr;
    u32Addr = ((u32Buffer)>>5);//256bit MIU
    HalUtilityW2BYTEMSK(REG_VIP_WDR1_7E_L,u32Addr,0xFFFF);
    HalUtilityW2BYTEMSK(REG_VIP_WDR1_7F_L,(u32Addr>>16),0x7FF);
}
void HalSclVipSetNrHistOnOff(bool bEn)
{
    HalUtilityW2BYTEMSK(REG_VIP_NE_19_L,bEn ?  BIT0: 0,BIT0);
}
s32 HalSclVipGetWdrHistogram(HalSclVipRoiType_e enType,u16 idx)
{
    s32 s32Y = 0;
    u32 u32Reg = REG_SCL_ARBSHP_09_L + 4*enType;
    switch(idx)
    {
        case 0:
            s32Y = HalUtilityR4BYTEDirect(u32Reg);
            break;
        case 1:
            s32Y = HalUtilityR4BYTEDirect(u32Reg+36);
            break;
        case 2:
            s32Y = HalUtilityR4BYTEDirect(u32Reg+72);
            break;
        case 3:
            s32Y = HalUtilityR4BYTEDirect(u32Reg+108);
            break;
        default:
            SCL_ERR("[HALSCLVIP]%s idx not support\n",__FUNCTION__);
            SCLOS_BUG();
            break;
    }
    return s32Y;
}
void HalSclVipSetWDRMaskOnOff(bool bEn)
{
    HalUtilityW2BYTEMSK(REG_SCL_ARBSHP_00_L,bEn ?BIT6 :0,BIT6);
}
void HalSclVipSetNRMaskOnOff(bool bEn)
{
    HalUtilityW2BYTEMSK(REG_VIP_NE_19_L,bEn ?0 : BIT1,BIT1);  //0 close update
}
void HalSclVipSetNRHistogramYCSel(HalSclVipYCSelType_e enType)
{
    switch(enType)
    {
        case E_HAL_SCLVIP_YCSel_Y:
            HalUtilityCloseShadow();
            HalUtilityW2BYTEMSKDirect(REG_VIP_NE_18_L,0,BIT0);
            HalUtilityOpenShadow();
            break;
        case E_HAL_SCLVIP_YCSel_C:
            HalUtilityCloseShadow();
            HalUtilityW2BYTEMSKDirect(REG_VIP_NE_18_L,BIT0,BIT0);
            HalUtilityOpenShadow();
            break;
        default:
            SCL_ERR("[HALSCLVIP]%s enType not support\n",__FUNCTION__);
            break;
    }
}
void HalSclVipGetNRHistogramDebug(void *pvCfg)
{
#if defined(SCLOS_TYPE_LINUX_DEBUG)
    u16 *u16pBuf;
    u16 idx;
    u16pBuf = (u16 *)pvCfg;
    SCL_ERR("[HALVIP] %s\n",__FUNCTION__);
    for(idx = 0;idx<NRBUFFERCNT;idx++)
    {
        SCL_ERR("[HALVIP] %hd:%hx @%lx\n",idx,*(u16pBuf+idx),(u32)(u16pBuf+idx));
        SCL_ERR("[HALVIP] %hd:%hx\n",idx,*(u16pBuf+((idx*2)+NRBUFFERCNT)));
        SCL_ERR("[HALVIP] %hd:%hx\n",idx,*(u16pBuf+((idx*2)+NRBUFFERCNT)+1));
        SCL_ERR("[HALVIP] %hd:%hx\n",idx,*(u16pBuf+(idx+(NRBUFFERCNT*3))));
    }
#endif
}
void HalSclVipGetNRDummy(void *pvCfg)
{
    u16 *u16pBuf;
    u16pBuf = (u16 *)pvCfg;
    *(u16pBuf) = HalUtilityR2BYTEDirect(REG_VIP_NE_10_L);
    *(u16pBuf+1) = HalUtilityR2BYTEDirect(REG_VIP_NE_16_L);
    *(u16pBuf+2) = HalUtilityR2BYTEDirect(REG_VIP_NE_70_L);
    *(u16pBuf+3) = HalUtilityR2BYTEDirect(REG_VIP_NE_71_L);
    *(u16pBuf+4) = HalUtilityR2BYTEDirect(REG_VIP_NE_72_L);
}
void HalSclVipGetNRHistogram(void *pvCfg)
{
    u16 *u16pBuf;
    u16 idx;
    u16pBuf = (u16 *)pvCfg;
    for(idx = 0;idx<NRBUFFERCNT;idx++)
    {
        *(u16pBuf+idx) = HalUtilityR2BYTEDirect(REG_VIP_NE_30_L+(idx*2));
        *(u16pBuf+((idx*2)+NRBUFFERCNT)) = HalUtilityR2BYTEDirect(REG_VIP_NE_38_L+(idx*4));
        *(u16pBuf+((idx*2)+NRBUFFERCNT)+1) = HalUtilityR2BYTEDirect(REG_VIP_NE_38_L+(idx*4)+2);
        *(u16pBuf+(idx+(NRBUFFERCNT*3))) = HalUtilityR2BYTEDirect(REG_VIP_NE_48_L+(idx*2));
    }
    //HalSclVipGetNRHistogramDebug(pvCfg);
}
void HalSclVipSetDlcShift(u8 u8value)
{
    HalUtilityW2BYTEMSK(REG_VIP_DLC_03_L,u8value,0x0007);
}
void HAlSclVipSetDlcMode(u8 u8value)
{
    HalUtilityW2BYTEMSK(REG_VIP_MWE_1C_L,u8value<<2,0x0004);
}
void HalSclVipSetDlcActWin(bool bEn,u16 u16Vst,u16 u16Hst,u16 u16Vnd,u16 u16Hnd)
{
    HalUtilityW2BYTEMSK(REG_VIP_DLC_08_L,bEn<<7,0x0080);
    HalUtilityW2BYTEMSK(REG_VIP_MWE_01_L,u16Vst,0x03FF);
    HalUtilityW2BYTEMSK(REG_VIP_MWE_02_L,u16Vnd,0x03FF);
    HalUtilityW2BYTEMSK(REG_VIP_MWE_03_L,u16Hst,0x01FF);
    HalUtilityW2BYTEMSK(REG_VIP_MWE_04_L,u16Hnd,0x01FF);
}
void HalSclVipDlcHistSetRange(u8 u8value,u8 u8range)
{
    u16 u16tvalue;
    u16 u16Mask;
    u32 u32Reg;
    u8range = u8range-1;
    u32Reg = REG_VIP_DLC_0C_L+(((u8range)/2)*2);
    if((u8range%2) == 0)
    {
        u16Mask     = 0x00FF;
        u16tvalue   = ((u16)u8value);

    }
    else
    {
        u16Mask     = 0xFF00;
        u16tvalue   = ((u16)u8value)<<8;
    }

    HalUtilityW2BYTEMSK(u32Reg,(u16tvalue),u16Mask);

}
u32 HalSclVipDlcHistGetRange(u8 u8range)
{
    u32 u32tvalue;
    u32 u32Reg;
    u32Reg      = REG_VIP_MWE_20_L+(((u8range)*2)*2);
    u32tvalue   = HalUtilityR4BYTEDirect(u32Reg);
    return u32tvalue;

}

u8 HalSclVipDlcGetBaseIdx(void)
{
    u8 u8tvalue;
    u8tvalue = HalUtilityR2BYTEDirect(REG_VIP_MWE_15_L);
    u8tvalue = (u8)(u8tvalue&0x80)>>7;
    return u8tvalue;
}

u32 HalSclVipDlcGetPC(void)
{
    u32 u32tvalue;
    u32tvalue = HalUtilityR4BYTEDirect(REG_VIP_MWE_08_L);
    return u32tvalue;
}

u32 HalSclVipDlcGetPW(void)
{
    u32 u32tvalue;
    u32tvalue = HalUtilityR4BYTEDirect(REG_VIP_MWE_0A_L);
    return u32tvalue;
}

u8 HalSclVipDlcGetMinP(void)
{
    u16 u16tvalue;
    u16tvalue = HalUtilityR2BYTEDirect(REG_VIP_DLC_62_L);
    u16tvalue = (u16tvalue>>8);
    return (u8)u16tvalue;
}

u8 HalSclVipDlcGetMaxP(void)
{
    u16 u16tvalue;
    u16tvalue = HalUtilityR2BYTEDirect(REG_VIP_DLC_62_L);
    return (u8)u16tvalue;
}

void HalSclVipSetAutoDownloadAddr(u32 u32baseadr,u16 u16iniaddr,u8 u8cli)
{
    switch(u8cli)
    {
        case 9:
            HalUtilityW2BYTEMSK(REG_SCL1_73_L, (u16)(u32baseadr>>4), 0xFFFF);
            HalUtilityW2BYTEMSK(REG_SCL1_74_L, (u16)(u32baseadr>>20), 0x01FF);
            HalUtilityW2BYTEMSK(REG_SCL1_77_L, ((u16)u16iniaddr), 0xFFFF);
            break;
        default:
            break;
    }
}

void HalSclVipSetAutoDownloadReq(u16 u16depth,u16 u16reqlen,u8 u8cli)
{
    switch(u8cli)
    {
        case 9:
            HalUtilityW2BYTEMSK(REG_SCL1_76_L, ((u16)u16reqlen), 0xFFFF);
            HalUtilityW2BYTEMSK(REG_SCL1_75_L, ((u16)u16depth), 0xFFFF);
            break;
        default:
            break;
    }
}

void HalSclVipSetAutoDownload(u8 bCLientEn,u8 btrigContinue,u8 u8cli)
{
    switch(u8cli)
    {
        case 9:
            HalUtilityW2BYTEMSK(REG_SCL1_72_L, bCLientEn|(btrigContinue<<1), 0x0003);
            break;
        default:
            break;
    }
}

void HalSclVipSetAutoDownloadTimer(u8 bCLientEn)
{
    HalUtilityW2BYTEMSK(REG_SCL1_78_L, bCLientEn<<15, 0x8000);
}
void HalSclVipGetNlmSram(u16 u16entry)
{
    u32 u32tvalue1,u32tvalue2;
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_62_L, u16entry, 0x07FF);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_62_L, 0x8000, 0x8000);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_62_L, 0x2000, 0x2000);
    u32tvalue1 = HalUtilityR2BYTEDirect(REG_SCL_NLM0_64_L);
    u32tvalue2 = HalUtilityR2BYTEDirect(REG_SCL_NLM0_65_L);
    u32tvalue1 |= ((u32tvalue2&0x00F0)<<12);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_62_L, 0x0000, 0x8000);
    SCL_DBG(SCL_DBG_LV_DRVVIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[Get_SRAM]entry%hx :%lx\n",u16entry,u32tvalue1);
}

void HalSclVipSetNlmSrambyCPU(u16 u16entry,u32 u32tvalue)
{
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_62_L, u16entry, 0x07FF);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_62_L, 0x8000, 0x8000);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_63_L, (u16)u32tvalue, 0xFFFF);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_65_L, (u16)(u32tvalue>>16), 0x000F);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_62_L, 0x4000, 0x4000);
    HalUtilityW2BYTEMSK(REG_SCL_NLM0_62_L, 0x0000, 0x8000);
    SCL_DBG(SCL_DBG_LV_DRVVIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[Set_SRAM]entry%hx :%lx\n",u16entry,u32tvalue);
}
void HalSclVipInitNeDummy(void)
{
    HalUtilityW2BYTEMSKDirect(REG_VIP_NE_10_L, 38400, 0xFFFF);
    HalUtilityW2BYTEMSKDirect(REG_VIP_NE_16_L, 6, 0x000F);
    HalUtilityW2BYTEMSKDirect(REG_VIP_NE_70_L, 0x2010, 0xFFFF);
    HalUtilityW2BYTEMSKDirect(REG_VIP_NE_71_L, 0x0220, 0xFFFF);
    HalUtilityW2BYTEMSKDirect(REG_VIP_NE_72_L, 0x0404, 0xFFFF);
}
void HalSclVipSetVpsSRAMEn(bool bEn)
{
    HalUtilityW2BYTEMSK(REG_VIP_PK_10_L, bEn ? BIT7 : 0, BIT7);
}
void HalSclVipInitUCMReqLenTh(void)
{
    HalUtilityW2BYTEMSKDirect(REG_UCM1_20_L, 0x1070, 0xFFFF);
    HalUtilityW2BYTEMSKDirect(REG_UCM1_21_L, 0x1070, 0xFFFF);
    HalUtilityW2BYTEMSKDirect(REG_UCM1_22_L, 0x7070, 0xFFFF);
    HalUtilityW2BYTEMSKDirect(REG_UCM1_23_L, 0x1010 , 0xFFFF);
}
void HalSclVipInitY2R(void)
{
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP2_60_L, 0x0A01, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP2_61_L, 0x59E, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP2_62_L, 0x401, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP2_63_L, 0x1FFF, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP2_64_L, 0x1D24, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP2_65_L, 0x400, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP2_66_L, 0x1E9F, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP2_67_L, 0x1FFF, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP2_68_L, 0x400, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP2_69_L, 0x719, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP1_6C_L, 0x181, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP1_6D_L, 0x1FF, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP1_6E_L, 0x1E54, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP1_6F_L, 0x1FAD, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP1_70_L, 0x132, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP1_71_L, 0x259, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP1_72_L, 0x75, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP1_73_L, 0x1F53, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP1_74_L, 0x1EAD, 0x1FFF);
    HalUtilityW2BYTEMSKDirect(REG_SCL_HVSP1_75_L, 0x1FF, 0x1FFF);
}
#if defined(SCLOS_TYPE_LINUX_DEBUG)
bool HalSclVipGetVipBypass(void)
{
    bool bRet;
    bRet = HalUtilityR2BYTE(REG_VIP_LCE_70_L)&BIT0;
    return bRet;
}
bool HalSclVipGetMcnrBypass(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_VIP_MCNR_01_L)&BIT1)? 0: 1;
    return bRet;
}
bool HalSclVipGetNlmBypass(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_SCL_NLM0_01_L)&BIT0)? 0: 1;
    return bRet;
}
bool HalSclVipGetAckBypass(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_VIP_ACE_38_L)&BIT6)? 0: 1;
    return bRet;
}
bool HalSclVipGetIbcBypass(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_VIP_ACE2_10_L)&BIT7)? 0: 1;
    return bRet;
}
bool HalSclVipGetIccBypass(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_VIP_ACE3_60_L)&BIT6)? 0: 1;
    return bRet;
}
bool HalSclVipGetIhcBypass(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_VIP_ACE2_24_L)&BIT7)? 0: 1;
    return bRet;
}
u16 HalSclVipGetFccBypass(void)
{
    u16 bRet = 0;
    bRet = (HalUtilityR2BYTE(REG_VIP_ACE_10_L)&BIT0)? 0: 1;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_ACE_10_L)&BIT1)? 0: 1)<<1;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_ACE_10_L)&BIT2)? 0: 1)<<2;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_ACE_10_L)&BIT3)? 0: 1)<<3;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_ACE_10_L)&BIT4)? 0: 1)<<4;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_ACE_10_L)&BIT5)? 0: 1)<<5;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_ACE_10_L)&BIT6)? 0: 1)<<6;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_ACE_10_L)&BIT7)? 0: 1)<<7;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_ACE_10_L)&BIT8)? 0: 1)<<8;
    return bRet;
}
bool HalSclVipGetUvcBypass(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_VIP_DLC_08_L)&BIT8)? 0: 1;
    return bRet;
}
bool HalSclVipGetDlcBypass(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_VIP_DLC_04_L)&BIT7)? 0: 1;
    bRet |= ((HalUtilityR2BYTE(REG_VIP_DLC_04_L)&BIT1)? 0: 1)<<1;
    return bRet;
}
bool HalSclVipGetLceBypass(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_VIP_LCE_10_L)&BIT0)? 0: 1;
    return bRet;
}
bool HalSclVipGetPkBypass(void)
{
    bool bRet;
    bRet = (HalUtilityR2BYTE(REG_VIP_PK_10_L)&BIT0)? 0: 1;
    return bRet;
}
#endif
