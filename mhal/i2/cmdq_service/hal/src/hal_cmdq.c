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
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
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

///////////////////////////////////////////////////////////////////////////////
/////////////////////
// file   halCMDQ.c
// @brief  CMDQ HAL
// @author MStar Semiconductor,Inc.
///////////////////////////////////////////////////////////////////////////////
/////////////////////
#if 0
#include <linux/types.h>
#endif
#include "cam_os_wrapper.h"
#include "drv_cmdq_os.h"
#include "hal_cmdq_dbg.h"
#include "hal_cmdq_util.h"
//#include "drv_cmdq.h"
#include "hal_cmdq.h"

#ifdef HAL_K6_SIMULATE

//#define REG_CMDQCTRL_BASE         0x46A00UL //0x123500 * 2  -> 0x246A00 (0xBF246A00)
u32 gHalRegCmdCtlBase[NUMBER_OF_CMDQ_HW] =
{
    0x123500UL
};

#elif defined(HAL_I3_SIMULATE)
u32 gHalRegCmdCtlBase[NUMBER_OF_CMDQ_HW] =
{
    0x112300UL
};

#else

//#define REG_CMDQCTRL_BASE         0x46A00UL //0x123500 * 2  -> 0x246A00 (0xBF246A00)
u32 gHalRegCmdCtlBase[NUMBER_OF_CMDQ_HW] =
{

    0x123500UL,
    0x123c00UL,
    0x123d00UL,
    0x123e00UL,
    0x122f00UL
};
#endif

static u32                      _gu32RiuBase[NUMBER_OF_CMDQ_HW] = { 0 };
REG_CMDQCtrl             *_CMDQCtrl[NUMBER_OF_CMDQ_HW] = { NULL };

#define REG32_W(reg, value)     do {    \
        (reg)->H = ((value) >> 16); \
        (reg)->L = ((value) & 0x0000FFFFUL);  \
    } while(0)

//---------------------------------------------------------------------------
//  Macro of bit operations
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
///concatenate (reg)->H and (reg)->L
/// @param  reg                     \b IN: concatenate data
//---------------------------------------------------------------------------
u32 _CMDQ_REG32_R(REG32 *reg)
{
    u32     value;
    value = (reg)->H << 16;
    value |= (reg)->L;
    return value;
}

//---------------------------------------------------------------------------
//  Inline Function
//---------------------------------------------------------------------------

void HAL_CMDQ_SetBank(HALCmdqIPSupport_e eIpNum, u32 u32BankAddr)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    _gu32RiuBase[eIpNum] = u32BankAddr;

    _CMDQCtrl[eIpNum] = (REG_CMDQCtrl*)(_gu32RiuBase[eIpNum] + (gHalRegCmdCtlBase[eIpNum] * 2));
    CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, " first cmdq ip(%d) is 0x%08x-0x%x\n", eIpNum, (u32)_CMDQCtrl[eIpNum], *(volatile u32*)(_gu32RiuBase[eIpNum] +     0x247204UL));

#ifdef HAL_K6_SIMULATE
    *(volatile u32*)(_gu32RiuBase[eIpNum] + 0x247204UL) = 0x0000;  // 0x2 47204, Enable CMDQ wirte RIU  0x247204 >> 1 ==> 0x123902
    // bank 0x1239 h0001 set bit_12 to be 0(cmdq host access RIU will be secure, so that cmdq can
    // access secure/non-secure bank)
#endif

#if  !defined(HAL_K6_SIMULATE) && !defined(HAL_I3_SIMULATE)
    /*write 0x100117 bit 0 as 1 to enable riu mask write function*/
    W2BYTEMSK(_gu32RiuBase[eIpNum],0x10012e,0x1,0x1);
#endif
    CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, "cmdq ip(%d) is 0x%08x-0x%x\n", eIpNum, (u32)_CMDQCtrl[eIpNum], *(volatile u32*)(_gu32RiuBase[eIpNum] +  0x247204UL));
}
//---------------------------------------------------------------------------
///set the element of _CMDQCtrl
///  .CMDQ_Enable
///  .CMDQ_Length_ReadMode
///  .CMDQ_Mask_Setting
//---------------------------------------------------------------------------
void HAL_CMDQ_Enable(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Enable), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Enable) | (CMDQ_CMDQ_EN));
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Length_ReadMode), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Length_ReadMode) | (CMDQ_REQ_LEN_MASK) | (CMDQ_REQ_TH_MASK) | (REQ_DMA_BURST_MODE));
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Mask_Setting), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Mask_Setting) | (CMDQ_MASK_BIT));

    // do not jump wait/polling_eq/polling_neq command while timer reaches,very important, set to 0
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Tout_Base_Amount),  (TOUT_DO_NOT_JUMP));
}

void HAL_CMDQ_Enable_TriggerBusSample(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Wait_Trig), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Wait_Trig) | (CMDQ_EN_TRIG_SAMPLE));
}

void HAL_CMDQ_Stop(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Enable), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Enable)& ~(CMDQ_CMDQ_EN));
}

void HAL_CMDQ_Reset(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_En_Clk_Miu), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_En_Clk_Miu) | (CMDQ_SOFT_RSTZ));
}

bool HAL_CMDQ_Set_Mode(HALCmdqIPSupport_e eIpNum, u32 ModeSel)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    if(ModeSel == 1)
    {
        REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode) | (CMDQ_CMD_BUF_DIRECT_MODE));
        return true;
    }
    else if(ModeSel == 0)
    {
        REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode) | (CMDQ_CMD_INCREAMENT_MODE));
        return true;
    }
    else if(ModeSel == 4)
    {
        REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode) | (CMDQ_RING_BUFFER_MODE));
        return true;
    }
    else
    {
        CMDQ_ERR("\033[35mHAL_CMDQ_Set_Mode ERROR!! Unknown mode, ModeSel = %d\033[m\n", ModeSel); // joe.liu
        return false;
    }
    return false;
}

void HAL_CMDQ_Set_Start_Pointer(HALCmdqIPSupport_e eIpNum, u32 StartAddr)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Cmd_St_Ptr), StartAddr);
    CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, "\033[35mset SATRT_ADDR: 0x%x\033[m\n", (u32)StartAddr); // joe.liu
}

void HAL_CMDQ_Set_End_Pointer(HALCmdqIPSupport_e eIpNum, u32 EndAddr)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Cmd_End_Ptr), EndAddr);
    CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, "\033[35mset END_ADDR: 0x%x\033[m\n", EndAddr);
// joe.liu
}

void HAL_CMDQ_Set_Offset_Pointer(HALCmdqIPSupport_e eIpNum, u32 OffsetAddr)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Cmd_End_Ptr), OffsetAddr);
    CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, "\033[35mset END_ADDR: 0x%x\033[m\n", OffsetAddr);
}

void HAL_CMDQ_Set_Timer(HALCmdqIPSupport_e eIpNum, u32 time)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Poll_Ratio_Wait_Time), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Poll_Ratio_Wait_Time) | time);
    CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, "\033[35mSet Timer: 0x%X\033[m\n", (unsigned int)time); // joe.liu
}

void HAL_CMDQ_Set_Ratio(HALCmdqIPSupport_e eIpNum, u32 Ratio)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Poll_Ratio_Wait_Time), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Poll_Ratio_Wait_Time) | Ratio);
    CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, "\033[35mSet Ratio: 0x%X\033[m\n", (unsigned int)Ratio); // joe.liu
}

void HAL_CMDQ_SetISRMSK(HALCmdqIPSupport_e eIpNum, u32 nMakVal)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Irq_Mask_Irq_Force),(_CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Irq_Mask_Irq_Force)&0xffff) | (nMakVal << 16));
}

u16 HAL_CMDQ_GetISRMSK(HALCmdqIPSupport_e eIpNum)
{
    u16 nRetVal;
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    nRetVal = (_CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Irq_Mask_Irq_Force))>>16;
    return nRetVal;
}

void HAL_CMDQ_Reset_Soft_Interrupt(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Sw_Trig_Cap_Sel_Irq_Clr), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Sw_Trig_Cap_Sel_Irq_Clr) | (CMDQ_SOFT_INTER_CLR));
}

void HAL_CMDQ_Reset_Start_Pointer_bit(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode) | (CMDQ_RST_CMD_ST_PTR_TRIG));
}

void HAL_CMDQ_Read_Soft_Interrupt(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Raw_Irq_Final_Irq);
}

bool HAL_CMDQ_Read_Idle_Done(HALCmdqIPSupport_e eIpNum)
{
    u32 nIrqStat = 0;
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
        return false;
    }

    nIrqStat = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Raw_Irq_Final_Irq);

    if(nIrqStat & CMDQ_READ_IDLE_DONE)
        return true;

    return false;
}

void HAL_CMDQ_Clear_IRQByFlag(HALCmdqIPSupport_e eIpNum, u16 u16IRQ)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Sw_Trig_Cap_Sel_Irq_Clr), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Sw_Trig_Cap_Sel_Irq_Clr) | (u16IRQ));
}

u16 HAL_CMDQ_Read_Irq_Status(HALCmdqIPSupport_e eIpNum)
{
    u32 nIrqStat = 0;
    u16 nRtnStatus;
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
        return false;
    }

    nIrqStat = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Raw_Irq_Final_Irq);
    /*read raw irq 0x45*/
    nRtnStatus = nIrqStat >> 16;
    //nRtnStatus = nIrqStat >> 16;
    //nRtnStatus = nRtnStatus | (nIrqStat& 0xff);

    return nRtnStatus;
}

u16 HAL_CMDQ_Read_FinalIrq_Status(HALCmdqIPSupport_e eIpNum)
{
    u32 nIrqStat = 0;
    u16 nRtnStatus;
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
        return false;
    }

    nIrqStat = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Raw_Irq_Final_Irq);
    /*read raw irq 0x45*/
    //nRtnStatus = nIrqStat >> 16;
    //nRtnStatus = nIrqStat >> 16;
    nRtnStatus = nIrqStat & 0xffff;

    return nRtnStatus;
}

bool HAL_CMDQ_Read_Dec_Done(HALCmdqIPSupport_e eIpNum)
{
    u32 nIrqStat = 0;
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
        return false;
    }

    nIrqStat = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Raw_Irq_Final_Irq);

    if(nIrqStat & CMDQ_READ_DEC_DONE)
        return true;

    return false;
}

//---------------------------------------------------------------------------
///Trigger for update start pointer and end pointer
//---------------------------------------------------------------------------
void HAL_CMDQ_Start(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode) | (CMDQ_MOV_CMD_PTR));
}

void HAL_CMDQ_Print_All(HALCmdqIPSupport_e eIpNum)
{
#if 0
    CMDQ_LOG("0x%08x-0x%08x-0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Enable, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode
           , (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_St_Ptr, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_End_Ptr);
    CMDQ_LOG("0x%08x-0x%08x-0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Sw_Wr_Mi_Wadr, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Rd_Mi_Radr
           , (u32)&_CMDQCtrl[eIpNum]->CMDQ_Buf_Radr_Wadr, (u32)&_CMDQCtrl[eIpNum]->CMDQ_0F_0E);

    CMDQ_LOG("0x%08x-0x%08x-0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Length_ReadMode, (u32)&_CMDQCtrl[eIpNum]->CMDQ_1f_12
           , (u32)&_CMDQCtrl[eIpNum]->CMDQ_Mask_Setting, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Wait_Trig);
    CMDQ_LOG("0x%08x-0x%08x-0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Force_Skip, (u32)&_CMDQCtrl[eIpNum]->CMDQ_27_26
           , (u32)&_CMDQCtrl[eIpNum]->CMDQ_Tout_Base_Amount, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Mode_Enable);

    CMDQ_LOG("0x%08x-0x%08x-0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_One_Step_Trig, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Replace_Dat2_3
           , (u32)&_CMDQCtrl[eIpNum]->CMDQ_En_Clk_Miu, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Debug_Sel);
    CMDQ_LOG("0x%08x-0x%08x-0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_3f_34, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Dma_State_Rb_Cmd
           , (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Sel_Decode_State, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Raw_Irq_Final_Irq);

    CMDQ_LOG("0x%08x-0x%08x-0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Irq_Mask_Irq_Force, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Sw_Trig_Cap_Sel_Irq_Clr
           , (u32)&_CMDQCtrl[eIpNum]->CMDQ_Poll_Ratio_Wait_Time, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Ptr_Vld);
    CMDQ_LOG("0x%08x-0x%08x-0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_75_4e, (u32)&_CMDQCtrl[eIpNum]->CMDQ_Hw_dummy
           , (u32)&_CMDQCtrl[eIpNum]->CMDQ_dummy, (u32)&_CMDQCtrl[eIpNum]->CMDQ_dummy);
#endif
    CMDQ_LOG("*************************************************************\n");

    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Enable, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Enable));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_St_Ptr, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_St_Ptr));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_End_Ptr, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_End_Ptr));

    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Sw_Wr_Mi_Wadr, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Sw_Wr_Mi_Wadr));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Rd_Mi_Radr, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Rd_Mi_Radr));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Buf_Radr_Wadr, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Buf_Radr_Wadr));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_0F_0E, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_0F_0E));

    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Length_ReadMode, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Length_ReadMode));
    //printk("0x%08x-0x%08x\n",(u32)&_CMDQCtrl[eIpNum]->CMDQ_1f_12,_CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_1f_12));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Mask_Setting, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Mask_Setting));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Wait_Trig, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Wait_Trig));

    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Force_Skip, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Force_Skip));
    //printk("0x%08x-0x%08x\n",(u32)&_CMDQCtrl[eIpNum]->CMDQ_27_26,_CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_27_26));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Tout_Base_Amount, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Tout_Base_Amount));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Mode_Enable, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Mode_Enable));

    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_One_Step_Trig, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_One_Step_Trig));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Replace_Dat2_3, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Replace_Dat2_3));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_En_Clk_Miu, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_En_Clk_Miu));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Debug_Sel, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Debug_Sel));

    //printk("0x%08x-0x%08x\n",(u32)&_CMDQCtrl[eIpNum]->CMDQ_3f_34,_CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_3f_34));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Dma_State_Rb_Cmd, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Dma_State_Rb_Cmd));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Sel_Decode_State, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Sel_Decode_State));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Raw_Irq_Final_Irq, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Raw_Irq_Final_Irq));

    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Irq_Mask_Irq_Force, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Irq_Mask_Irq_Force));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Sw_Trig_Cap_Sel_Irq_Clr, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Sw_Trig_Cap_Sel_Irq_Clr));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Poll_Ratio_Wait_Time, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Poll_Ratio_Wait_Time));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Ptr_Vld, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Ptr_Vld));

    CMDQ_LOG("tb final irq = 0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Final_Irq, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Final_Irq));
    CMDQ_LOG("tb irq force = 0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Force_Mask, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Force_Mask));
    CMDQ_LOG("tb irq clr = 0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Irq_Clr, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Irq_Clr));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Hw_dummy, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Hw_dummy));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_dummy, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_dummy));
    CMDQ_LOG("0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_dummy, _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_dummy));


}

u32 HAL_CMDQ_Read_Dummy_Register(HALCmdqIPSupport_e eIpNum)
{

    u32 reg_value = 0;

    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
        return 0;
    }

    reg_value = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_dummy) & 0xffff;

    return reg_value;
}

s32 HAL_CMDQ_Write_Dummy_Register(HALCmdqIPSupport_e eIpNum, u32 DummyValue)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
        return -1;
    }

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_dummy), (_CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_dummy) & (CMDQ_DUMMY_WRITE_ZERO)) | DummyValue);
    return 0;
}

s32 HAL_CMDQ_Clear_HwDummy_Register(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
        return -1;
    }

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Hw_dummy), 0x0);
    return 0;
}

void HAL_CMDQ_Set_MIU_SELECT(HALCmdqIPSupport_e eIpNum, u32 miu_select)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    if(miu_select == 1)
    {
        CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, "\033[35mset miu_1\033[m\n");
        REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Length_ReadMode), (_CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Length_ReadMode) | CMDQ_MIU_SELECT_MIU1));
    }
    else
    {
        CMDQ_DBG(CMDQ_DBG_LVL0_TYPE, "\033[35mset miu_0\033[m\n");
        REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Length_ReadMode), (_CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Length_ReadMode) & ~(CMDQ_MIU_SELECT_MIU1)));
    }
}

u32 HAL_CMDQ_Read_Start_Pointer(HALCmdqIPSupport_e eIpNum)
{
    u32 reg_value = 0;

    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    reg_value = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_St_Ptr);
    return reg_value;
}

u32 HAL_CMDQ_Read_End_Pointer(HALCmdqIPSupport_e eIpNum)
{
    u32 reg_value = 0;
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }


    reg_value = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_End_Ptr);
    return reg_value;
}

u32 HAL_CMDQ_Get_Dummy_Register_RiuAddr(HALCmdqIPSupport_e eIpNum)
{
    u32 nRetAdr;
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    nRetAdr = gHalRegCmdCtlBase[eIpNum] | (CMDQ_DUMMY_REG_OFFSET * 2);
    //CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "dummy register address 0x%08x\n", (u32)nRetAdr);
    return (u32)nRetAdr;
}

u32 HAL_CMDQ_Get_TriggerClr_Register_RiuAddr(HALCmdqIPSupport_e eIpNum)
{
    u32 nRetAdr;
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    nRetAdr = gHalRegCmdCtlBase[eIpNum] | (CMDQ_TRIBUS_CLR_REG_OFFSET * 2);
    CMDQ_DBG(CMDQ_DBG_LVL2_TYPE, "trigger bus register address 0x%08x\n", nRetAdr);
    return nRetAdr;
}

//---------------------------------------------------------------------------
///Set Previous Dummy Register bit to be 1(which means this CAF is already write to DRAM)
//---------------------------------------------------------------------------
void HAL_CMDQ_Write_Dummy_Register_release_polling(HALCmdqIPSupport_e eIpNum, u32 Write_bit)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_dummy), SET_FLAG1(_CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_dummy), 0x0001 << Write_bit));
}

u32 HAL_CMDQ_Error_Command(HALCmdqIPSupport_e eIpNum, u32 select_bit)
{
    u32 reg_value = 0;

    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Sel_Decode_State), select_bit);
    reg_value = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Dma_State_Rb_Cmd) & 0x0000ffff;
    return reg_value;
}

void HAL_CMDQ_Write_Pointer(HALCmdqIPSupport_e eIpNum, u32 Write_value)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Sw_Wr_Mi_Wadr), Write_value);
}
//---------------------------------------------------------------------------
///Set rd_mi_radr_trig to grab current read address pointer(rd_mi_radr will be store in CMDQ_Rd_Mi_Radr)
//---------------------------------------------------------------------------
u32 HAL_CMDQ_Read_Pointer(HALCmdqIPSupport_e eIpNum)
{
    u32 reg_value = 0;
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
        return 0;
    }
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode), _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode) | CMDQ_READ_TRIG);
    reg_value = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Rd_Mi_Radr);

    return reg_value;
}

//---------------------------------------------------------------------------
///get the Write_Pointer(for multi-process, preventing write address is wrong(each process will start from Buffer_Start) ==> get current write_cmd ptr,next will write from here
//---------------------------------------------------------------------------
u32 HAL_CMDQ_Get_Write_Pointer(HALCmdqIPSupport_e eIpNum)
{
    u32 reg_value = 0;

    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    reg_value = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Sw_Wr_Mi_Wadr);
    return reg_value;
}

//---------------------------------------------------------------------------
///Force trigger signal on wait bus. (good for debug), current sigbits 0-15
//---------------------------------------------------------------------------
void HAL_CMDQ_Trigger_Wait(HALCmdqIPSupport_e eIpNum, u32 sigbits)
{
    u32 v;

    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    v = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Wait_Trig) & 0xFFFF0000;
    v = v | (sigbits & 0x0000FFFF);

    //@FIXME, do we need skip mask here. or add individual function to set it.
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Wait_Trig), v);
    //@FIXME, do we need to recover skip mask bit, if we set above 2 line.
}

//---------------------------------------------------------------------------
///Skip WR[0] WAIT[1], POLLEQ[2], POLLNEQ[3] commands. EX. 0xF skip those 4 cmds.
//---------------------------------------------------------------------------
void HAL_CMDQ_Skip_Commands(HALCmdqIPSupport_e eIpNum, u32 skipbits)
{
    u32 v;
    //@FIXME, r0a0, active low, does it really active low???
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }
    //we also set skip mask bit, to force skip even mask is not set.
    v = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Force_Skip);
    if(skipbits & 0x01)
        v |= 0x00000011;
    else if(skipbits & 0x02)
        v |= 0x00000022;
    else if(skipbits & 0x03)
        v |= 0x00000044;
    else if(skipbits & 0x04)
        v |= 0x00000088;

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Force_Skip), v);
}
//---------------------------------------------------------------------------
///Real CMDQ clear trigger bus irq
//---------------------------------------------------------------------------
void HAL_CMDQ_Clear_TiggerBusIrq(HALCmdqIPSupport_e eIpNum, u16 nClrVal)
{
    u32 nVal;

    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    nVal = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode);

    nVal |= nClrVal;
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Irq_Clr), nVal);

}
//---------------------------------------------------------------------------
///Real CMDQ read trigger bus final irq
//---------------------------------------------------------------------------
u16 HAL_CMDQ_Read_TiggerBusFinalIrq(HALCmdqIPSupport_e eIpNum)
{
    u32 nVal;
    u16 nRtnStatus;

    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    nVal = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Final_Irq);
    nRtnStatus = nVal & 0xffff;
    return nRtnStatus;

}

//---------------------------------------------------------------------------
///Real CMDQ reset
//---------------------------------------------------------------------------
void HAL_CMDQ_Reset2(HALCmdqIPSupport_e eIpNum, bool high)
{
    u32 v;

    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    v = _CMDQ_REG32_R(&_CMDQCtrl[eIpNum]->CMDQ_En_Clk_Miu);
    if(high)
        REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_En_Clk_Miu), v | (CMDQ_SOFT_RSTZ_BIT));
    else
    {
        REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_En_Clk_Miu), v & (~CMDQ_SOFT_RSTZ_BIT));
    }

}

//---------------------------------------------------------------------------
///Read misc status bits.
//---------------------------------------------------------------------------
u32 HAL_CMDQ_Read_Misc_Status(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    return _CMDQ_REG32_R((&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Ptr_Vld));
}

void HAL_CMDQ_Set_Debug_Step_Mode(HALCmdqIPSupport_e eIpNum, bool on)
{
    u32 r;

    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    r = _CMDQ_REG32_R((&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Mode_Enable));
    if(on)
        r |= (CMDQ_DEBUG_MODE_ENABLE | CMDQ_DEBUG_ONESTEP_ENABLE);
    else
        r &= ~(CMDQ_DEBUG_MODE_ENABLE | CMDQ_DEBUG_ONESTEP_ENABLE);
    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Mode_Enable), r);
}

u32 HAL_CMDQ_Get_Debug_Step_Mode(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    return _CMDQ_REG32_R((&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Mode_Enable));
}

//---------------------------------------------------------------------------
///Trigger one step when debug-mode, step-mode enabled.
//---------------------------------------------------------------------------
void HAL_CMDQ_Debug_One_Step(HALCmdqIPSupport_e eIpNum)
{
    if(eIpNum >= NUMBER_OF_CMDQ_HW)
    {
        CMDQ_ERR("%s no support CMDQ IP(%d)\n", __func__, eIpNum);
    }

    REG32_W((&_CMDQCtrl[eIpNum]->CMDQ_One_Step_Trig),
            _CMDQ_REG32_R((&_CMDQCtrl[eIpNum]->CMDQ_One_Step_Trig)) | CMDQ_DEBUG_ONESTEP);
}
