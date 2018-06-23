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
#define __HAL_GOP_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "cam_os_wrapper.h"
#if defined(CAM_OS_RTK)
#include "cam_os_wrapper.h"
#include <string.h>
#else
#include <linux/string.h>
#endif

#include "hal_gop_util.h"
#include "hal_gop.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define HAL_GOP_DEBUG   0

#if HAL_GOP_DEBUG
#if defined(CAM_OS_RTK)
#define HALGOPDBG(fmt, arg...) UartSendTrace(fmt, ##arg) //CamOsDebug(fmt, ##arg)
#define HALGOPERR(fmt, arg...) UartSendTrace(fmt, ##arg) //CamOsDebug(fmt, ##arg)
#else
#define HALGOPDBG(fmt, arg...) printk(KERN_INFO fmt, ##arg) //CamOsDebug(fmt, ##arg)
#define HALGOPERR(fmt, arg...) printk(KERN_INFO fmt, ##arg) //CamOsDebug(fmt, ##arg)
#endif
#else
#if defined(CAM_OS_RTK)
#define HALGOPERR(fmt, arg...) UartSendTrace(fmt, ##arg) //CamOsDebug(fmt, ##arg)
#define HALGOPDBG(fmt, arg...)
#else
#define HALGOPERR(fmt, arg...) printk(KERN_INFO fmt, ##arg) //CamOsDebug(fmt, ##arg)
#define HALGOPDBG(fmt, arg...)
#endif
#endif

#define PARSING_ALPHA_TYPE(bConst)(\
                                   bConst == 0 ? "PIXEL_ALPHA": \
                                   bConst == 1 ? "CONST_ALPHA": \
                                   "Wrong_Alpha_Type")

#define PARSING_DISPLAY_MODE(x) (\
                                 x == E_HAL_GOP_DISPLAY_MODE_INTERLACE ? "INTERLACE" : \
                                 x == E_HAL_GOP_DISPLAY_MODE_PROGRESS  ? "PROGRESS"  : \
                                 "UNKNOWN")

#define PARSING_OUT_MODE(x) (\
                             x == E_HAL_GOP_OUT_FMT_RGB ? "RGB" : \
                             x == E_HAL_GOP_OUT_FMT_YUV ? "YUV" : \
                             "UNKNOWIN")

#define PARSING_SRC_FMT(x) (\
                            x == E_HAL_GOP_GWIN_SRC_RGB1555    ? "RGB1555"  :   \
                            x == E_HAL_GOP_GWIN_SRC_RGB565     ? "RGB565"   :   \
                            x == E_HAL_GOP_GWIN_SRC_ARGB4444   ? "ARGB4444" :   \
                            x == E_HAL_GOP_GWIN_SRC_2266       ? "2266"     :   \
                            x == E_HAL_GOP_GWIN_SRC_I8_PALETTE ? "I8_PALETTE" : \
                            x == E_HAL_GOP_GWIN_SRC_ARGB8888   ? "ARGB8888" :   \
                            x == E_HAL_GOP_GWIN_SRC_ARGB1555   ? "ARGB1555" :   \
                            x == E_HAL_GOP_GWIN_SRC_ABGR8888   ? "ABGR8888" :   \
                            x == E_HAL_GOP_GWIN_SRC_UV7Y8      ? "YV7Y8"    :   \
                            x == E_HAL_GOP_GWIN_SRC_UV8Y8      ? "UV8Y8"    :   \
                            x == E_HAL_GOP_GWIN_SRC_RGBA5551   ? "TGBA5551" :   \
                            x == E_HAL_GOP_GWIN_SRC_RGBA4444   ? "RGBA444"  :   \
                            "UNKNOWN")

#define IsHalGOPBaseIdType_0(x)          (x == REG_ISPGOP_00_BASE)
#define IsHalGOPBaseIdType_1(x)          (x == REG_ISPGOP_10_BASE)
#define IsHalGOPBaseIdType_2(x)          (x == REG_ISPGOP_20_BASE)
#define IsHalGOPBaseIdType_3(x)          (x == REG_ISPGOP_30_BASE)
#define IsHalGOPBaseIdType_4(x)          (x == REG_DIPGOP_40_BASE)
#define IsHalGOPBaseIdType_5(x)          (x == REG_DISPGOP_50_BASE)
#define IsHalGOPBaseIdType_6(x)          (x == REG_DISPGOP_60_BASE)
#define IsHalGWINBaseIdType_0(x)          (x == REG_ISPGOP_01_BASE)
#define IsHalGWINBaseIdType_1(x)          (x == REG_ISPGOP_11_BASE)
#define IsHalGWINBaseIdType_2(x)          (x == REG_ISPGOP_21_BASE)
#define IsHalGWINBaseIdType_3(x)          (x == REG_ISPGOP_31_BASE)
#define IsHalGWINBaseIdType_4(x)          (x == REG_DIPGOP_41_BASE)
#define IsHalGWINBaseIdType_5(x)          (x == REG_DISPGOP_51_BASE)
#define IsHalGWINBaseIdType_6(x)          (x == REG_DISPGOP_61_BASE)

#define GOP_WORD_BASE 0
#define GOP_PIXEL_BASE 1

//if set 1, use cmdq to set register, if set 0, use cpu to set register
#define GOP_REG_CMDQ_MD 1

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

MHAL_CMDQ_CmdqInterface_t _tGopCmdqLocal[E_HAL_GOP_CMDQ_ID_NUM];

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

HalGopWrRegType_e _eGopWrRegMd = E_HAL_GOP_FORCE_WR;


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

void _HalGopWrite2Byte(u32  u32Reg, u16 u16Val,HalGopCmdqIdType_e eCmdqId)
{
    if(eCmdqId < E_HAL_GOP_CMDQ_ID_NUM && GOP_REG_CMDQ_MD && _tGopCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdq)
    {
        _tGopCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdq(&_tGopCmdqLocal[eCmdqId],u32Reg,u16Val);
    }
    else
    {
        W2BYTE(u32Reg, u16Val);
    }
}

void _HalGopWrite2ByteMsk(u32 u32Reg, u16 u16Val, u16 u16Mask,HalGopCmdqIdType_e eCmdqId)
{
    if(eCmdqId < E_HAL_GOP_CMDQ_ID_NUM && GOP_REG_CMDQ_MD && _tGopCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdqMask)
    {
        _tGopCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdqMask(&_tGopCmdqLocal[eCmdqId],u32Reg,u16Val,u16Mask);
    }
    else
    {
        W2BYTEMSK(u32Reg, u16Val, u16Mask);
    }
}
u16 _HalGopGetDoubleRW(HalGopIdType_e eGopId)
{
    u16 u16DoubleRW;


    if(_eGopWrRegMd==E_HAL_GOP_FORCE_WR)
    {
        u16DoubleRW = GOP_BANK_FORCE_WR;
    }
    else
    {
    u16DoubleRW = GOP_BANK_DOUBLE_WR_G00;
    }
    /*
    switch(eGopId)
    {
        case E_HAL_GOP_ID_00:
        case E_HAL_GOP_ID_01:
        case E_HAL_GOP_ID_0_ST:
            u16DoubleRW = GOP_BANK_DOUBLE_WR_G00;
            break;
        case E_HAL_GOP_ID_10:
        case E_HAL_GOP_ID_11:
        case E_HAL_GOP_ID_1_ST:
            u16DoubleRW = GOP_BANK_DOUBLE_WR_G00;
            break;
        case E_HAL_GOP_ID_20:
        case E_HAL_GOP_ID_21:
        case E_HAL_GOP_ID_2_ST:
            u16DoubleRW = GOP_BANK_DOUBLE_WR_G00;
            break;
        default:
            u16DoubleRW = GOP_BANK_DOUBLE_WR_G00;
            HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
            break;
    }
    */
    return u16DoubleRW;

}

u32 _HalGopGetBaseAddr(HalGopIdType_e eGopId)
{
    u32 u32Base;

    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
            u32Base = REG_ISPGOP_00_BASE;
            break;
        case E_HAL_ISPGOP_ID_01:
            u32Base = REG_ISPGOP_01_BASE;
            break;
        case E_HAL_ISPGOP_ID_0_ST:
            u32Base = REG_ISPGOP_0ST_BASE;
            break;
        case E_HAL_ISPGOP_ID_10:
            u32Base = REG_ISPGOP_10_BASE;
            break;
        case E_HAL_ISPGOP_ID_11:
            u32Base = REG_ISPGOP_11_BASE;
            break;
        case E_HAL_ISPGOP_ID_1_ST:
            u32Base = REG_ISPGOP_1ST_BASE;
            break;
        case E_HAL_ISPGOP_ID_20:
            u32Base = REG_ISPGOP_20_BASE;
            break;
        case E_HAL_ISPGOP_ID_21:
            u32Base = REG_ISPGOP_21_BASE;
            break;
        case E_HAL_ISPGOP_ID_2_ST:
            u32Base = REG_ISPGOP_2ST_BASE;
            break;
        case E_HAL_ISPGOP_ID_30:
            u32Base = REG_ISPGOP_30_BASE;
            break;
        case E_HAL_ISPGOP_ID_31:
            u32Base = REG_ISPGOP_31_BASE;
            break;
        case E_HAL_ISPGOP_ID_3_ST:
            u32Base = REG_ISPGOP_3ST_BASE;
            break;
        case E_HAL_DIPGOP_ID_40:
            u32Base = REG_DIPGOP_40_BASE;
            break;
        case E_HAL_DIPGOP_ID_41:
            u32Base = REG_DIPGOP_41_BASE;
            break;
        case E_HAL_DIPGOP_ID_4_ST:
            u32Base = REG_DIPGOP_4ST_BASE;
            break;
        case E_HAL_DISPGOP_ID_50:
            u32Base = REG_DISPGOP_50_BASE;
            break;
        case E_HAL_DISPGOP_ID_51:
            u32Base = REG_DISPGOP_51_BASE;
            break;
        case E_HAL_DISPGOP_ID_5_ST:
            u32Base = REG_DISPGOP_5ST_BASE;
            break;
        case E_HAL_DISPGOP_ID_60:
            u32Base = REG_ISPGOP_20_BASE;
            break;
        case E_HAL_DISPGOP_ID_61:
            u32Base = REG_DISPGOP_61_BASE;
            break;
        case E_HAL_DISPGOP_ID_6_ST:
            u32Base = REG_DISPGOP_6ST_BASE;
            break;
        default:
            u32Base = REG_ISPGOP_00_BASE;
            HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
            break;

    }
    return u32Base;
}

u32 _HalGopGwinRegBaseShift(HalGopGwinIdType_e eGwinId)
{
    u32 u32RegBaseShift;

    switch(eGwinId)
    {
        case E_HAL_GOP_GWIN_ID_0:
            u32RegBaseShift = REG_GWIN0_REGBASE_OFFSET;
            break;
        case E_HAL_GOP_GWIN_ID_1:
            u32RegBaseShift = REG_GWIN1_REGBASE_OFFSET;
            break;
        default:
            u32RegBaseShift = REG_GWIN0_REGBASE_OFFSET;
            HALGOPERR("[GOP]%s %d: wrong GwinId\n", __FUNCTION__, __LINE__);
            break;

    }
    return u32RegBaseShift;
}

u16 _HalGopGetMiuAlign(HalGopIdType_e eGopId)
{
    u16 u16MiuAlign;

    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
        case E_HAL_ISPGOP_ID_01:
        case E_HAL_ISPGOP_ID_0_ST:
        case E_HAL_ISPGOP_ID_10:
        case E_HAL_ISPGOP_ID_11:
        case E_HAL_ISPGOP_ID_1_ST:
        case E_HAL_ISPGOP_ID_20:
        case E_HAL_ISPGOP_ID_21:
        case E_HAL_ISPGOP_ID_2_ST:
        case E_HAL_ISPGOP_ID_30:
        case E_HAL_ISPGOP_ID_31:
        case E_HAL_ISPGOP_ID_3_ST:
        case E_HAL_DIPGOP_ID_40:
        case E_HAL_DIPGOP_ID_41:
        case E_HAL_DIPGOP_ID_4_ST:
            u16MiuAlign = MIU_BUS_ALIGN_GOP_INFINITY;
            break;
        case E_HAL_DISPGOP_ID_50:
        case E_HAL_DISPGOP_ID_51:
        case E_HAL_DISPGOP_ID_5_ST:
        case E_HAL_DISPGOP_ID_60:
        case E_HAL_DISPGOP_ID_61:
        case E_HAL_DISPGOP_ID_6_ST:
            u16MiuAlign = MIU_BUS_ALIGN_GOP_K6;
            break;
        default:
            u16MiuAlign = MIU_BUS_ALIGN_GOP_INFINITY;
            HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
            break;
    }

    return u16MiuAlign;

}

u16 _HalGopGetPipeDelay(HalGopIdType_e eGopId)
{
    u16 u16PipeDelay;

    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
        case E_HAL_ISPGOP_ID_01:
        case E_HAL_ISPGOP_ID_0_ST:
        case E_HAL_ISPGOP_ID_10:
        case E_HAL_ISPGOP_ID_11:
        case E_HAL_ISPGOP_ID_1_ST:
        case E_HAL_ISPGOP_ID_20:
        case E_HAL_ISPGOP_ID_21:
        case E_HAL_ISPGOP_ID_2_ST:
        case E_HAL_ISPGOP_ID_30:
        case E_HAL_ISPGOP_ID_31:
        case E_HAL_ISPGOP_ID_3_ST:
        case E_HAL_DIPGOP_ID_40:
        case E_HAL_DIPGOP_ID_41:
        case E_HAL_DIPGOP_ID_4_ST:
            u16PipeDelay = 0;
            break;
        case E_HAL_DISPGOP_ID_50:
        case E_HAL_DISPGOP_ID_51:
        case E_HAL_DISPGOP_ID_5_ST:
        case E_HAL_DISPGOP_ID_60:
        case E_HAL_DISPGOP_ID_61:
        case E_HAL_DISPGOP_ID_6_ST:
            u16PipeDelay = 0;// not update
            break;
        default:
            u16PipeDelay = 0;
            HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
            break;
    }

    return u16PipeDelay;

}

u32 _HalGopGetScalerEnableGopRegAddr(HalGopIdType_e eGopId)
{
    u32 u32Base;

    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
        case E_HAL_ISPGOP_ID_01:
        case E_HAL_ISPGOP_ID_0_ST:
            u32Base = REG_SCL_EN_ISPGOP_00;
            break;
        case E_HAL_ISPGOP_ID_10:
        case E_HAL_ISPGOP_ID_11:
        case E_HAL_ISPGOP_ID_1_ST:
            u32Base = REG_SCL_EN_ISPGOP_01;
            break;
        case E_HAL_ISPGOP_ID_20:
        case E_HAL_ISPGOP_ID_21:
        case E_HAL_ISPGOP_ID_2_ST:
            u32Base = REG_SCL_EN_ISPGOP_02;
            break;
        case E_HAL_ISPGOP_ID_30:
        case E_HAL_ISPGOP_ID_31:
        case E_HAL_ISPGOP_ID_3_ST:
            u32Base = REG_SCL_EN_ISPGOP_03;
            break;
         case E_HAL_DIPGOP_ID_40:
        case E_HAL_DIPGOP_ID_41:
        case E_HAL_DIPGOP_ID_4_ST:
            u32Base = REG_DIP_EN_DIPGOP_04;
            break;
        default:
            u32Base = REG_SCL_EN_ISPGOP_00;
            HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
            break;

    }
    return u32Base;
}

bool _HalGopIdTransCmdqID(HalGopIdType_e eGopId,HalGopCmdqIdType_e *pCmdqId)
{
    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
        case E_HAL_ISPGOP_ID_01:
        case E_HAL_ISPGOP_ID_0_ST:
        case E_HAL_ISPGOP_ID_10:
        case E_HAL_ISPGOP_ID_11:
        case E_HAL_ISPGOP_ID_1_ST:
        case E_HAL_ISPGOP_ID_20:
        case E_HAL_ISPGOP_ID_21:
        case E_HAL_ISPGOP_ID_2_ST:
        case E_HAL_ISPGOP_ID_30:
        case E_HAL_ISPGOP_ID_31:
        case E_HAL_ISPGOP_ID_3_ST:
            *pCmdqId = E_HAL_GOP_CMDQ_VPE_ID_0;
            return TRUE;
        case E_HAL_DIPGOP_ID_40:
        case E_HAL_DIPGOP_ID_41:
        case E_HAL_DIPGOP_ID_4_ST:
            *pCmdqId =  E_HAL_GOP_CMDQ_DIVP_ID_1;
            return TRUE;;
        default:
            *pCmdqId =  E_HAL_GOP_CMDQ_ID_NUM;
            HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
            break;

    }
    return FALSE;
}

u16 _HalGopGetBytePerPixel(HalGopGwinSrcFormat_e eSrcType)
{
    u16 u16BytePerPixel = 1;
    switch(eSrcType)
    {
        case E_HAL_GOP_GWIN_SRC_RGB1555:
        case E_HAL_GOP_GWIN_SRC_RGB565:
        case E_HAL_GOP_GWIN_SRC_ARGB4444:
        case E_HAL_GOP_GWIN_SRC_ARGB1555:
        case E_HAL_GOP_GWIN_SRC_RGBA5551:
        case E_HAL_GOP_GWIN_SRC_RGBA4444:
            u16BytePerPixel = 2;
            break;

        case E_HAL_GOP_GWIN_SRC_I8_PALETTE:
        case E_HAL_GOP_GWIN_SRC_I4_PALETTE:
        case E_HAL_GOP_GWIN_SRC_I2_PALETTE:
            u16BytePerPixel = 1;
            break;

        case E_HAL_GOP_GWIN_SRC_ARGB8888:
        case E_HAL_GOP_GWIN_SRC_ABGR8888:
            u16BytePerPixel = 4;
            break;

        case E_HAL_GOP_GWIN_SRC_2266:
        case E_HAL_GOP_GWIN_SRC_UV7Y8:
        case E_HAL_GOP_GWIN_SRC_UV8Y8:
            u16BytePerPixel = 2;
            break;

    }
    return u16BytePerPixel;
}

bool _HalGopGetSclFclkStatus(HalGopIdType_e eGopId)
{
    bool bSclFclkEn = 1;

    // not update
    /*
    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
        case E_HAL_ISPGOP_ID_01:
        case E_HAL_ISPGOP_ID_0_ST:
        case E_HAL_ISPGOP_ID_10:
        case E_HAL_ISPGOP_ID_11:
        case E_HAL_ISPGOP_ID_1_ST:
            bSclFclkEn = !R2BYTEMSK(REG_SCL_FCLK_BASE + REG_GOP_64, 0x0001);//fclk1: 0=Enable, 1=Disable
            break;

        case E_HAL_ISPGOP_ID_20:
        case E_HAL_ISPGOP_ID_21:
        case E_HAL_ISPGOP_ID_2_ST:
            bSclFclkEn = !R2BYTEMSK(REG_SCL_FCLK_BASE + REG_GOP_65, 0x0001);//fclk2: 0=Enable, 1=Disable
            break;
        default:
            bSclFclkEn = !R2BYTEMSK(REG_SCL_FCLK_BASE + REG_GOP_64, 0x0001);//fclk1: 0=Enable, 1=Disable
            break;

    }
    */
    return bSclFclkEn;
}

void _HalGopSetSclFclkStatus(HalGopIdType_e eGopId, bool bEn)
{
    HalGopCmdqIdType_e eCmdqId;
    // not update

    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
        case E_HAL_ISPGOP_ID_01:
        case E_HAL_ISPGOP_ID_0_ST:
        case E_HAL_ISPGOP_ID_10:
        case E_HAL_ISPGOP_ID_11:
        case E_HAL_ISPGOP_ID_1_ST:
            _HalGopIdTransCmdqID(eGopId,&eCmdqId);
            _HalGopWrite2ByteMsk(REG_SCL_FCLK_BASE + REG_GOP_64, !bEn, 0x0001,eCmdqId); //fclk1: 0=Enable, 1=Disable
            break;

        case E_HAL_ISPGOP_ID_20:
        case E_HAL_ISPGOP_ID_21:
        case E_HAL_ISPGOP_ID_2_ST:
            _HalGopIdTransCmdqID(eGopId,&eCmdqId);
            _HalGopWrite2ByteMsk(REG_SCL_FCLK_BASE + REG_GOP_65, !bEn, 0x0001,eCmdqId); //fclk2: 0=Enable, 1=Disable
            break;
        default:
            _HalGopIdTransCmdqID(eGopId,&eCmdqId);
            _HalGopWrite2ByteMsk(REG_SCL_FCLK_BASE + REG_GOP_64, !bEn, 0x0001,eCmdqId); //fclk1: 0=Enable, 1=Disable
            break;

    }
}

u32 _HalGopGetOsdbBaseAddr(HalGopIdType_e eGopId)
{
    u32 u32Base;

    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
        case E_HAL_ISPGOP_ID_01:
        case E_HAL_ISPGOP_ID_0_ST:
        case E_HAL_ISPGOP_ID_10:
        case E_HAL_ISPGOP_ID_11:
        case E_HAL_ISPGOP_ID_1_ST:
            u32Base = REG_SCL_TO_GOP_BASE;
            break;
        case E_HAL_ISPGOP_ID_20:
        case E_HAL_ISPGOP_ID_21:
        case E_HAL_ISPGOP_ID_2_ST:
        case E_HAL_ISPGOP_ID_30:
        case E_HAL_ISPGOP_ID_31:
        case E_HAL_ISPGOP_ID_3_ST:
            u32Base = REG_SCL_TO_GOP_BASE2;
            break;
        case E_HAL_DIPGOP_ID_40:
        case E_HAL_DIPGOP_ID_41:
        case E_HAL_DIPGOP_ID_4_ST:
            u32Base = REG_DIP_TO_GOP_BASE;
            break;
        case E_HAL_DISPGOP_ID_50:
        case E_HAL_DISPGOP_ID_51:
        case E_HAL_DISPGOP_ID_5_ST:
        case E_HAL_DISPGOP_ID_60:
        case E_HAL_DISPGOP_ID_61:
        case E_HAL_DISPGOP_ID_6_ST:
            u32Base = REG_DISPSCL_TO_GOP_BASE;
            break;
        default:
            u32Base = REG_SCL_TO_GOP_BASE;
            HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
            break;

    }
    return u32Base;
}

u32 _HalGopOsdbRegBaseShift(HalGopIdType_e eGopId)
{
    u32 u32RegBaseShift;

    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
        case E_HAL_ISPGOP_ID_01:
        case E_HAL_ISPGOP_ID_0_ST:
            u32RegBaseShift = REG_SCL_EN_ISPGOP_00;
            break;
        case E_HAL_ISPGOP_ID_10:
        case E_HAL_ISPGOP_ID_11:
        case E_HAL_ISPGOP_ID_1_ST:
            u32RegBaseShift = REG_SCL_EN_ISPGOP_01;
            break;
        case E_HAL_ISPGOP_ID_20:
        case E_HAL_ISPGOP_ID_21:
        case E_HAL_ISPGOP_ID_2_ST:
            u32RegBaseShift = REG_SCL_EN_ISPGOP_02;
            break;
        case E_HAL_ISPGOP_ID_30:
        case E_HAL_ISPGOP_ID_31:
        case E_HAL_ISPGOP_ID_3_ST:
            u32RegBaseShift = REG_SCL_EN_ISPGOP_03;
            break;
        case E_HAL_DIPGOP_ID_40:
        case E_HAL_DIPGOP_ID_41:
        case E_HAL_DIPGOP_ID_4_ST:
            u32RegBaseShift = REG_DIP_EN_DIPGOP_04;
            break;
        case E_HAL_DISPGOP_ID_50:
        case E_HAL_DISPGOP_ID_51:
        case E_HAL_DISPGOP_ID_5_ST:
        case E_HAL_DISPGOP_ID_60:
        case E_HAL_DISPGOP_ID_61:
        case E_HAL_DISPGOP_ID_6_ST:
            u32RegBaseShift = REG_DISPSCL_EN_DISPGOP_05_VOP;
            break;
        default:
            u32RegBaseShift = REG_SCL_EN_ISPGOP_00;
            HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
            break;

    }
    return u32RegBaseShift;
}
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void HalGopInitCmdq(void)
{
    memset(&_tGopCmdqLocal[0],0,sizeof(MHAL_CMDQ_CmdqInterface_t)*E_HAL_GOP_CMDQ_ID_NUM);
    HALGOPDBG("[GOP]%s %d: out \n", __FUNCTION__, __LINE__);
}
void HalGopSetCmdq(MHAL_CMDQ_CmdqInterface_t *pstCmdq,HalGopCmdqIdType_e eHalCmdqId)
{
    if(eHalCmdqId >= E_HAL_GOP_CMDQ_ID_NUM)
    {
        HALGOPERR("[GOP]%s %d: wrong CMDQId\n", __FUNCTION__, __LINE__);
        return ;
    }

    if(pstCmdq)
    {
        memcpy(&_tGopCmdqLocal[eHalCmdqId],pstCmdq,sizeof(MHAL_CMDQ_CmdqInterface_t));
    }
    else
    {
        memset(&_tGopCmdqLocal[eHalCmdqId],0,sizeof(MHAL_CMDQ_CmdqInterface_t));
    }
    HALGOPDBG("[GOP]%s %d: out \n", __FUNCTION__, __LINE__);
}

void HalGopUpdateBase(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u32 u32BaseAddr)
{
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    u16 GOP_Reg_DB   = 0;
    u16 MIU_BUS;
    u32 u32TempVal;
    HalGopCmdqIdType_e eCmdqId;
    //u32 wait_count   = 0;

    //for pan_display

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    MIU_BUS = _HalGopGetMiuAlign(eGopId);

    HALGOPDBG("[GOP]%s %d: Id:%02x, in, addr=0x%x \n", __FUNCTION__, __LINE__, eGopId, u32BaseAddr);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    //base address register is at gwin
    if(IsHalGWINBaseIdType_0(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_1(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_2(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_3(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_4(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_5(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_6(GOP_Reg_Base))
    {
        u32TempVal = u32BaseAddr >> MIU_BUS; // 128-bit unit = 16 bytes
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_01, (u32TempVal & 0xFFFF),eCmdqId); //Ring block start Low address 0x21:16
        u32TempVal = u32TempVal >> 0x10;
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_02, (u32TempVal & 0xFFFF),eCmdqId); //Ring block start Hi address 0x22:6, page number

        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);

        /*while((R2BYTEMSK(GOP_Reg_Base + REG_GOP_7F, GOP_BANK_GOP0_WR_ACK_MSK) != 0)) //wait double buffer write ack
        {
            wait_count++;
            CamOsMsSleep(3);
            if(wait_count > 10)
            {
                break;
            }
        }*/

    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId for base addr settings\n", __FUNCTION__, __LINE__);
    }

    HALGOPDBG("[GOP]%s %d: out \n", __FUNCTION__, __LINE__);
}

// set gwin pitch with color format
void HalGopSetGwinMemPitch(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u32 u32MemPixelWidth, HalGopGwinSrcFormat_e eSrcFmt)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    u16 u16FrameBuffer_Bytes_Per_Pixel;
    u16 MIU_BUS;
    bool bGopBaseType;
    HalGopCmdqIdType_e eCmdqId;

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    u16FrameBuffer_Bytes_Per_Pixel = _HalGopGetBytePerPixel(eSrcFmt);
    MIU_BUS = _HalGopGetMiuAlign(eGopId);
    bGopBaseType = GOP_PIXEL_BASE;

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(IsHalGWINBaseIdType_0(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_1(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_2(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_3(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_4(GOP_Reg_Base))
    {

        bGopBaseType = GOP_WORD_BASE;
        if(bGopBaseType == GOP_PIXEL_BASE)
        {
            u16FrameBuffer_Bytes_Per_Pixel = 1;
        }
        // framebuffer pitch
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_09
                          , (u32MemPixelWidth * u16FrameBuffer_Bytes_Per_Pixel) >> MIU_BUS,eCmdqId); //bytes per line for gop framebuffer

        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else if(IsHalGWINBaseIdType_5(GOP_Reg_Base) || IsHalGWINBaseIdType_6(GOP_Reg_Base))
    {
        bGopBaseType = GOP_PIXEL_BASE;
        if(bGopBaseType == GOP_PIXEL_BASE)
        {
            u16FrameBuffer_Bytes_Per_Pixel = 1;
        }
        // framebuffer pitch
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_09, (u32MemPixelWidth * u16FrameBuffer_Bytes_Per_Pixel) >> MIU_BUS,eCmdqId); //bytes per line for gop framebuffer

        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }


}

// set gwin pitch directly without calcuting with color format
void HalGopSetGwinMemPitchDirect(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u32 u32MemPitch)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    u16 MIU_BUS;
    HalGopCmdqIdType_e eCmdqId;

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    MIU_BUS = _HalGopGetMiuAlign(eGopId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(IsHalGWINBaseIdType_0(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_1(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_2(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_3(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_4(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_5(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_6(GOP_Reg_Base))
    {
        // framebuffer pitch
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_09, (u32MemPitch >> MIU_BUS),eCmdqId); //bytes per line for gop framebuffer

        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }


}

void HalGopUpdateParam(HalGopIdType_e eGopId, HalGopParamConfig_t tParamCfg)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB = 0;
    u16 GOP_Pipe_Delay = 0;
    HalGopCmdqIdType_e eCmdqId;
    HALGOPDBG("[GOP]%s %d: Id:%02x, Disp:%s, Out:%s, (%d %d %d %d)\n",
              __FUNCTION__, __LINE__, eGopId,
              PARSING_DISPLAY_MODE(tParamCfg.eDisplayMode),
              PARSING_OUT_MODE(tParamCfg.eOutFormat),
              tParamCfg.tStretchWindow.u16X, tParamCfg.tStretchWindow.u16Y,
              tParamCfg.tStretchWindow.u16Width, tParamCfg.tStretchWindow.u16Height);

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GOP_Pipe_Delay = _HalGopGetPipeDelay(eGopId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGOPBaseIdType_0(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_1(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_2(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_3(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_4(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_5(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_6(GOP_Reg_Base))
    {
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_00, GOP_SOFT_RESET | GOP_FIELD_INV,eCmdqId); //reset
        if(GOP_Reg_Base == REG_DIPGOP_40_BASE)// DIP has to set Vsync inverse
        {
            _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_00, (tParamCfg.eDisplayMode << 3) | GOP_HS_MASK | GOP_ALPHA_INV | GOP_VS_INV | GOP_HS_INV | (((u16)tParamCfg.eOutFormat) << 10),eCmdqId); //set Progress mode; mask Hsync; alpha inverse; YUV output
        }
        else
        {
            _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_00, (tParamCfg.eDisplayMode << 3) | GOP_HS_MASK | GOP_ALPHA_INV | GOP_HS_INV | (((u16)tParamCfg.eOutFormat) << 10),eCmdqId); //set Progress mode; mask Hsync; alpha inverse; YUV output
        }
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_01, GOP_REGDMA_INTERVAL_START | GOP_REGDMA_INTERVAL_END,eCmdqId);
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_02, GOP_BLINK_DISABLE,eCmdqId); //close blink
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_0A, 0x0010,eCmdqId); //insert fake rdy between hs & valid rdy

        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_0E, (tParamCfg.tStretchWindow.u16Width >> 1) + 1,eCmdqId); //miu efficiency = Stretch Window H size (unit:2 pixel) /2 +1
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_0F, GOP_Pipe_Delay,eCmdqId); //Hsync input pipe delay
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_19, GOP_BURST_LENGTH_MASK, GOP_BURST_LENGTH_MAX,eCmdqId); // set gop dma burst length

        // GOP display area global settings
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_30, tParamCfg.tStretchWindow.u16Width >> 1,eCmdqId); // Stretch Window H size (unit:2 pixel)
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_31, tParamCfg.tStretchWindow.u16Height,eCmdqId); // Stretch window V size
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_32, tParamCfg.tStretchWindow.u16X,eCmdqId); // Stretch Window H coordinate
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_34, tParamCfg.tStretchWindow.u16Y,eCmdqId); // Stretch Window V coordinate
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_36, GOP_STRETCH_VRATIO,eCmdqId); // Stretch V ratio
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_38, GOP_STRETCH_HINI,eCmdqId); // Stretch H start value
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_39, GOP_STRETCH_VINI,eCmdqId); // Stretch V start value

        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}


void HalGopUpdateGwinParam(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopGwinParamConfig_t tParamCfg)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB = 0;
    u16 MIU_BUS;
    u32 u32BaseAddr;
    u32 GWIN_Reg_Shift_Base = 0;
    u16 u16FrameBuffer_Bytes_Per_Pixel;
    u32 u32TempVal;
    u16 u16DispImage_HStart, u16DispImage_HEnd;
    u16 u16DispImage_VStart, u16DispImage_VEnd;
    HalGopCmdqIdType_e eCmdqId;

    HALGOPDBG("[GOP]%s %d: Id:%02x, Src:%s, (%d %d %d %d), Base:%08x Offset(%04x, %08x)\n",
              __FUNCTION__, __LINE__, eGopId,
              PARSING_SRC_FMT(tParamCfg.eSrcFmt),
              tParamCfg.tDispWindow.u16X, tParamCfg.tDispWindow.u16Y,
              tParamCfg.tDispWindow.u16Width, tParamCfg.tDispWindow.u16Height,
              tParamCfg.u32BaseAddr,
              tParamCfg.u16Base_XOffset, tParamCfg.u32Base_YOffset);

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    u16FrameBuffer_Bytes_Per_Pixel = _HalGopGetBytePerPixel(tParamCfg.eSrcFmt);
    MIU_BUS        =  _HalGopGetMiuAlign(eGopId);

    u16DispImage_HStart = tParamCfg.tDispWindow.u16X;
    u16DispImage_VStart = tParamCfg.tDispWindow.u16Y;
    u16DispImage_HEnd   = tParamCfg.tDispWindow.u16X + tParamCfg.tDispWindow.u16Width;
    u16DispImage_VEnd   = tParamCfg.tDispWindow.u16Y + tParamCfg.tDispWindow.u16Height;

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGWINBaseIdType_0(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_1(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_2(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_3(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_4(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_5(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_6(GOP_Reg_Base))
    {
        // gwin global settings
        _HalGopWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_00, ((u8)tParamCfg.eSrcFmt) << 4, 0x00F0,eCmdqId); //set gop format; enable gwin; alpha mode initial


        // framebuffer settings
        // framebuffer starting address
        u32BaseAddr = tParamCfg.u32BaseAddr;
        u32BaseAddr = u32BaseAddr >> MIU_BUS; // 128-bit unit = 16 bytes
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_01, (u32BaseAddr & 0xFFFF),eCmdqId);
        u32BaseAddr = u32BaseAddr >> 0x10;
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_02, (u32BaseAddr & 0xFFFF),eCmdqId);

        // framebuffer pitch
        if(IsHalGWINBaseIdType_5(GOP_Reg_Base) || IsHalGWINBaseIdType_6(GOP_Reg_Base))
        {
            u16FrameBuffer_Bytes_Per_Pixel = 1;
        }
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_09, (tParamCfg.tDispWindow.u16Width * u16FrameBuffer_Bytes_Per_Pixel) >> MIU_BUS,eCmdqId); //bytes per line for gop framebuffer

        // framebuffer V start offset  (line)
        u32TempVal = tParamCfg.u32Base_YOffset & 0xFFFF;
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_0C, u32TempVal,eCmdqId);
        u32TempVal = tParamCfg.u32Base_YOffset >> 0x10;
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_0D, u32TempVal,eCmdqId);

        // framebuffer H start offset  (pixel)
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_0E, tParamCfg.u16Base_XOffset,eCmdqId);


        // GOP display settings

        u16FrameBuffer_Bytes_Per_Pixel = 1; //for i2, all Hstr & Hend are pixel base
        // GWIN display area in panel : : H start postion and end information
        u32TempVal = (u16DispImage_HStart * u16FrameBuffer_Bytes_Per_Pixel) >> MIU_BUS;
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_04, u32TempVal,eCmdqId); // H start
        u32TempVal = (u16DispImage_HEnd * u16FrameBuffer_Bytes_Per_Pixel) >> MIU_BUS;
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_05, u32TempVal,eCmdqId); // H end

        // GWIN  display area in panel : V start postion and end information
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_06, u16DispImage_VStart,eCmdqId); // V start line
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_08, u16DispImage_VEnd,eCmdqId); // V end line


        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }

}

void HalGopSetStretchWindowSize(HalGopIdType_e eGopId, HalGopWindowType_t tGwinCfg)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB = 0;
    HalGopCmdqIdType_e eCmdqId;

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGOPBaseIdType_0(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_1(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_2(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_3(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_4(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_5(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_6(GOP_Reg_Base))
    {
        // GOP global settings
        _HalGopWrite2Byte(GOP_Reg_Base+REG_GOP_0E,(tGwinCfg.u16Width >>1)+1,eCmdqId);//miu efficiency = Stretch Window H size (unit:2 pixel) /2 +1

        // GOP display area global settings
        _HalGopWrite2Byte(GOP_Reg_Base+REG_GOP_30,tGwinCfg.u16Width >>1,eCmdqId); //Stretch Window H size (unit:2 pixel)
        _HalGopWrite2Byte(GOP_Reg_Base+REG_GOP_31, tGwinCfg.u16Height,eCmdqId);    //Stretch window V size

        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }


}

void HalGopSetGwinSize(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopWindowType_t tGwinCfg, HalGopGwinSrcFormat_e eSrcFmt)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB = 0;
    u16 MIU_BUS;
    u32 u32TempVal;
    u32 GWIN_Reg_Shift_Base = 0;
    u16 u16FrameBuffer_Bytes_Per_Pixel;
    u16 u16DispImage_HStart, u16DispImage_HEnd;
    u16 u16DispImage_VStart, u16DispImage_VEnd;
    bool bGopBaseType = GOP_PIXEL_BASE;
    HalGopCmdqIdType_e eCmdqId;

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    if(bGopBaseType == GOP_WORD_BASE)
    {
        u16FrameBuffer_Bytes_Per_Pixel = _HalGopGetBytePerPixel(eSrcFmt);
    }
    else
    {
        u16FrameBuffer_Bytes_Per_Pixel = 1;
    }
    u16DispImage_HStart = tGwinCfg.u16X;
    u16DispImage_VStart = tGwinCfg.u16Y;
    u16DispImage_HEnd   = tGwinCfg.u16X + tGwinCfg.u16Width;
    u16DispImage_VEnd   = tGwinCfg.u16Y + tGwinCfg.u16Height;
    MIU_BUS        = _HalGopGetMiuAlign(eGopId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGWINBaseIdType_0(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_1(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_2(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_3(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_4(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_5(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_6(GOP_Reg_Base))
    {
        // GOP display settings
        // GWIN display area in panel : : H start postion and end information
        u32TempVal = (u16DispImage_HStart * u16FrameBuffer_Bytes_Per_Pixel);// >> MIU_BUS; I2 here is pixel base
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_04, u32TempVal,eCmdqId); // H start
        u32TempVal = (u16DispImage_HEnd * u16FrameBuffer_Bytes_Per_Pixel);// >> MIU_BUS; I2 here is pixel base
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_05, u32TempVal,eCmdqId); // H end

        // GWIN  display area in panel : V start postion and end information
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_06, u16DispImage_VStart,eCmdqId); // V start line
        _HalGopWrite2Byte(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_08, u16DispImage_VEnd,eCmdqId); // V end line

        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }


}

void HalGopSetGwinSrcFmt(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopGwinSrcFormat_e eSrcFmt)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    HalGopCmdqIdType_e eCmdqId;

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGWINBaseIdType_0(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_1(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_2(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_3(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_4(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_5(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_6(GOP_Reg_Base))
    {
        _HalGopWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_00, eSrcFmt<<4, 0x00F0,eCmdqId); //set gop source format;

        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }


}


void HalGopSetOutFormat(HalGopIdType_e eGopId, bool bYUVOutput)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB   = 0;
    HalGopCmdqIdType_e eCmdqId;

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);

    HALGOPDBG("[GOP]%s %d: Id:%02x, byuvoutput:%d\n", __FUNCTION__, __LINE__, eGopId, bYUVOutput);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGOPBaseIdType_0(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_1(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_2(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_3(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_4(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_5(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_6(GOP_Reg_Base))
    {
        if(bYUVOutput)
        {
            _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00, GOP_YUVOUT, GOP_YUVOUT,eCmdqId);

        }
        else
        {
            _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00, 0, GOP_YUVOUT,eCmdqId);
        }

        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}


void HalGopSetAlphaBlending(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool bConstantAlpha, u8 Alpha)
{
    u32 GOP_Reg_Base = 0;
    u32 GOP_Reg_Base_AlphaInv = 0;
    u16 GOP_Reg_DB   = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    HalGopCmdqIdType_e eCmdqId;

    // 1. GOP Set Alpha Blending
    // 2. pixel_alpha=1, constant_alpha=0
    // 3. Alpha: constant Alpha value


    HALGOPDBG("[GOP]%s %d: Id:%02x, bConstantAlpha:%d, Alpha:%x\n", __FUNCTION__, __LINE__, eGopId, bConstantAlpha, Alpha);

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);

    // Alpha inv, which is at GOP setting
    GOP_Reg_Base_AlphaInv = GOP_Reg_Base == REG_ISPGOP_01_BASE ? REG_ISPGOP_00_BASE : \
                            GOP_Reg_Base == REG_ISPGOP_11_BASE ? REG_ISPGOP_10_BASE : \
                            GOP_Reg_Base == REG_ISPGOP_21_BASE ? REG_ISPGOP_20_BASE : \
                            GOP_Reg_Base == REG_ISPGOP_31_BASE ? REG_ISPGOP_30_BASE : \
                            GOP_Reg_Base == REG_DIPGOP_41_BASE ? REG_DIPGOP_40_BASE : \
                            GOP_Reg_Base == REG_DISPGOP_51_BASE ? REG_DISPGOP_50_BASE : \
                            GOP_Reg_Base == REG_DISPGOP_61_BASE ? REG_DISPGOP_60_BASE : \
                            REG_ISPGOP_00_BASE;
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGWINBaseIdType_0(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_1(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_2(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_3(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_4(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_5(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_6(GOP_Reg_Base))
    {
        // Alpha inv, which is at GOP setting
        //_HalGopWrite2ByteMsk(GOP_Reg_Base_AlphaInv + REG_GOP_00, (bConstantAlpha ? 0x0000 : GOP_ALPHA_INV), GOP_ALPHA_INV); //set Alpha inverse when pixel Alpha
        //refine for OSDB
        _HalGopWrite2ByteMsk(GOP_Reg_Base_AlphaInv + REG_GOP_00, (bConstantAlpha ? GOP_ALPHA_INV : 0x0000), GOP_ALPHA_INV,eCmdqId);


        // constant or pixel Alpha
        _HalGopWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_00, (bConstantAlpha ? GOP_CONST_ALPHA_EN : GOP_PIXEL_ALPHA_EN), GOP_ALPHA_MASK,eCmdqId);

        // Alpha value
        _HalGopWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_0A, (bConstantAlpha ? Alpha : 0x0000), 0x00FF,eCmdqId); //available for constant Alpha; set 0 when pixel Alpha


        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}

void HalGopSetColorKey(HalGopIdType_e eGopId, bool bEn, u8 u8R, u8 u8G, u8 u8B)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB   = 0;
    HalGopCmdqIdType_e eCmdqId;

    // 1. GOP Set RGB Color Key
    // 2. bEn= 0: disable; 1: enable
    // 3. u8R: red color value; u8G: green color value; u8B: blue color value ; value range: 0~255 (0x0~0xFF)


    HALGOPDBG("[GOP]%s %d: Id:%02x, bEn:%d, (R=%x, G=%x, B=%x)\n", __FUNCTION__, __LINE__, eGopId, bEn, u8R, u8G, u8B);

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGOPBaseIdType_0(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_1(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_2(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_3(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_4(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_5(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_6(GOP_Reg_Base))
    {
        if(bEn)
        {
            _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00, GOP_RGB_TRANSPARENT_COLOR_ENABLE, GOP_RGB_TRANS_COLOR_EN,eCmdqId);
            _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_24, (u8B | (u8G << 8)), 0xFFFF,eCmdqId);
            _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_25,  u8R, 0x00FF,eCmdqId);
        }
        else
        {
            _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00, GOP_RGB_TRANSPARENT_COLOR_DISABLE, GOP_RGB_TRANS_COLOR_EN,eCmdqId);
        }

        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}

void HalGopSetPipeDelay(HalGopIdType_e eGopId, u8 delay)
{
    u16 pipe_delay   = delay;
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB   = 0;
    HalGopCmdqIdType_e eCmdqId;
    // 1. GOP Set Pipe Delay


    HALGOPDBG("[GOP]%s %d: Id:%02x, delay:%d\n", __FUNCTION__, __LINE__, eGopId, delay);

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGOPBaseIdType_0(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_1(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_2(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_3(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_4(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_5(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_6(GOP_Reg_Base))
    {
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_0F, pipe_delay,eCmdqId);

        // Double Buffer Write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}

void HalGopSetPaletteRiu(HalGopIdType_e eGopId, u8 u8Index, u8 u8A, u8 u8R, u8 u8G, u8 u8B)
{
    u32 GOP_Reg_Base = 0;
    bool bSclFclkEnSta = 0;
    HalGopCmdqIdType_e eCmdqId;
    // 1. GOP Set Palette by RIU Mode
    // 2. GOP Palette SRAM Clock should be opened
    // 3. Set Force Write
    // 4. Set RIU Mode
    // 5. Set Palette Index, A, R, G, B Values
    // 6. Trigger Palette Write
    // 7. Close Force Write

    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    bSclFclkEnSta = _HalGopGetSclFclkStatus(eGopId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGOPBaseIdType_0(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_1(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_2(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_3(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_4(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_5(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_6(GOP_Reg_Base))
    {
        // not update
        if(!bSclFclkEnSta)//if scl fclk doesn't open, open it;then close it after load palette
        {
            _HalGopSetSclFclkStatus(eGopId, 1);
        }
        //set force write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_BANK_FORCE_WR, GOP_BANK_FORCE_WR,eCmdqId);

        //set RIU mode
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_05, GOP_PALETTE_SRAM_CTRL_RIU, GOP_PALETTE_SRAM_CTRL_MASK,eCmdqId);

        //set palette value
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_03, u8G << 8 | u8B,eCmdqId);
        _HalGopWrite2Byte(GOP_Reg_Base + REG_GOP_04, u8A << 8 | u8R,eCmdqId);

        //set palette index
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_05, u8Index, GOP_PALETTE_TABLE_ADDRESS_MASK,eCmdqId);

        //write trigger
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_05, 0x0100, GOP_PALETTE_WRITE_ENABLE_MASK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_05, 0x0000, GOP_PALETTE_WRITE_ENABLE_MASK,eCmdqId);

        //close force write
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0x0000, GOP_BANK_FORCE_WR,eCmdqId);

        if(!bSclFclkEnSta)
        {
            _HalGopSetSclFclkStatus(eGopId, 0);
        }
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }

}

void HalGopSetEnableGwin(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool bEn)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB   = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    HalGopCmdqIdType_e eCmdqId;

    HALGOPDBG("[GOP]%s %d: Set GopId=%d GwinId=%d enable_GOP = [%d]\n", __FUNCTION__, __LINE__, eGopId, eGwinId, bEn);

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGWINBaseIdType_0(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_1(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_2(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_3(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_4(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_5(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_6(GOP_Reg_Base))
    {
        // enable/disable gwin
        _HalGopWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base  + REG_GOP_00, bEn ? GOP_GWIN_ENABLE : 0, GOP_GWIN_ENABLE,eCmdqId);

        //write gop register
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}

void HalGopSetScalerEnableGop(HalGopIdType_e eGopId, bool bEn)
{
    u32 GOP_Reg_Base = 0;
    u32 GOP_Reg_Addr = 0;
    u32 GOP_OSDB_Reg_Shift_Base = 0;
    HalGopCmdqIdType_e eCmdqId;

    HALGOPDBG("[GOP]%s %d: Set enable_GOP from scaler ID:%d = [%d]\n", __FUNCTION__, __LINE__, eGopId, bEn);

    GOP_Reg_Base = _HalGopGetOsdbBaseAddr(eGopId);
    //GOP_Reg_Addr = _HalGopGetScalerEnableGopRegAddr(eGopId);
    GOP_OSDB_Reg_Shift_Base = _HalGopOsdbRegBaseShift(eGopId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    HALGOPDBG("[GOP]%s %d: GOP_Reg_Base=0x%x, GOP_Reg_Addr=0x%x, GOP_OSDB_Reg_Shift_Base=0x%x\n", __FUNCTION__, __LINE__, GOP_Reg_Base, GOP_Reg_Addr, GOP_OSDB_Reg_Shift_Base);
    // enable/disable gop from scaler
    _HalGopWrite2ByteMsk(GOP_Reg_Base + GOP_Reg_Addr + GOP_OSDB_Reg_Shift_Base, bEn ? GOP_GWIN_ENABLE : 0, GOP_GWIN_ENABLE,eCmdqId);

    // if id is DIP, set DE mode ( osdb en: close DE mode (enable handshake mode); osdb disable: enable DE mode)
    if(GOP_Reg_Base == REG_DIP_TO_GOP_BASE)
    {
        _HalGopWrite2ByteMsk(GOP_Reg_Base + GOP_Reg_Addr + GOP_OSDB_Reg_Shift_Base, bEn ? 0 : DIP_DE_MD_EN, DIP_DE_MD_MASK,eCmdqId);
    }

}

void HalGopSetArgb1555Alpha(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopGwinArgb1555Def_e eAlphaType, MS_U8 u8AlphaVal)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Reg_DB   = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    HalGopCmdqIdType_e eCmdqId;

    HALGOPDBG("[GOP]%s %d: Set GopId=%d GwinId=%d, argb1555 alpha type=%d, val=%d\n", __FUNCTION__, __LINE__, eGopId, eGwinId, eAlphaType, u8AlphaVal);

    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    if(IsHalGWINBaseIdType_0(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_1(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_2(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_3(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_4(GOP_Reg_Base))
    {
        // enable/disable gwin

        if(eAlphaType==E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0)
        {
            _HalGopWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base  + REG_GOP_03, u8AlphaVal, GOP_GWIN_ARGB1555_ALPHA0_DEF_MSK,eCmdqId);
        }
        else if(eAlphaType==E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1)
        {
            _HalGopWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base  + REG_GOP_03, (u8AlphaVal<<8), GOP_GWIN_ARGB1555_ALPHA1_DEF_MSK,eCmdqId);
        }
        else
        {
            HALGOPERR("[GOP]%s %d: parameter wrong\n", __FUNCTION__, __LINE__);
        }

        //write gop register
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
        _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    }
    else if(IsHalGWINBaseIdType_5(GOP_Reg_Base) || IsHalGWINBaseIdType_6(GOP_Reg_Base))
    {
        HALGOPERR("[GOP]%s %d: GOPId=%d not support\n", __FUNCTION__, __LINE__, eGopId);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}

//set osdb bypass for sc2, let sc2 don't have gop0's drawing
//enable: sc2 don't have gop0's content; disable: sc2 have gop0's content
void HalGop0SetOsdBypassForSc2Enable(bool bEn)
{
    u32 GOP_Reg_Base = 0;
    HalGopIdType_e eGopId = E_HAL_ISPGOP_ID_00;
    HalGopCmdqIdType_e eCmdqId;

    HALGOPDBG("[GOP]%s %d: set osdb bypass mode, enable = [%d]\n", __FUNCTION__, __LINE__, bEn);

    GOP_Reg_Base = REG_SCL_TO_GOP_BASE;
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    // enable/disable gwin
    _HalGopWrite2ByteMsk(GOP_Reg_Base + REG_GOP_03, bEn ? OSDB_BYPASS_MD_EN : 0, OSDB_BYPASS_MD_MASK,eCmdqId);

}

void HalGopGetEnableGwin(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool *bEn)
{
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;

    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);

    if(IsHalGWINBaseIdType_0(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_1(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_2(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_3(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_4(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_5(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_6(GOP_Reg_Base))
    {
        if(R2BYTEMSK(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_00, GOP_GWIN_ENABLE))
        {
            *bEn  = 1;
        }
        else
        {
            *bEn  = 0;
        }
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
        *bEn  = 0;
    }

    HALGOPDBG("[GOP]%s %d: Get enable_GOP = [%d]\n", __FUNCTION__,  __LINE__, *bEn);

}

void HalGopGetScalerEnableGop(HalGopIdType_e eGopId, bool *bEn)
{
    u32 GOP_Reg_Base = 0;
    u32 GOP_Reg_Addr = 0;
    u32 GOP_OSDB_Reg_Shift_Base = 0;

    GOP_Reg_Base = _HalGopGetOsdbBaseAddr(eGopId);
    //GOP_Reg_Addr = _HalGopGetScalerEnableGopRegAddr(eGopId);
    GOP_OSDB_Reg_Shift_Base = _HalGopOsdbRegBaseShift(eGopId);

    if(R2BYTEMSK(GOP_Reg_Base + GOP_Reg_Addr + GOP_OSDB_Reg_Shift_Base, GOP_GWIN_ENABLE))
    {
        *bEn  = 1;
    }
    else
    {
        *bEn  = 0;
    }

    HALGOPDBG("[GOP]%s %d: Get enable_GOP from scaler = [%d]\n", __FUNCTION__,  __LINE__, *bEn);
}


void HalGopGetAlpha(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool *pbConstantAlpha, u8 *pu8ConstantAlphaValue)
{
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;

    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);

    if(IsHalGWINBaseIdType_0(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_1(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_2(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_3(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_4(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_5(GOP_Reg_Base) ||
        IsHalGWINBaseIdType_6(GOP_Reg_Base))
    {
        // constant or pixel alpha
        if(R2BYTEMSK(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_00, GOP_PIXEL_ALPHA_EN) == GOP_PIXEL_ALPHA_EN)
        {
            *pbConstantAlpha = 0; //pixel_alpha
        }
        else
        {
            *pbConstantAlpha = 1; //constant_alpha
        }

        // alpha value
        *pu8ConstantAlphaValue = (u8)R2BYTEMSK(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_0A, 0x00FF);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }

    HALGOPDBG("[GOP]%s %d: Alpha_type = [%s], Alpha_value = [%d]\n", __FUNCTION__, __LINE__, PARSING_ALPHA_TYPE(*pbConstantAlpha), *pu8ConstantAlphaValue);

}

void HalGopGetColorKey(HalGopIdType_e eGopId, bool *bEn, u8 *u8R, u8  *u8G, u8  *u8B)
{
    u32 GOP_Reg_Base;
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);

    if(IsHalGOPBaseIdType_0(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_1(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_2(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_3(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_4(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_5(GOP_Reg_Base) ||
        IsHalGOPBaseIdType_6(GOP_Reg_Base))
    {
        // enable/disable
        *bEn = (R2BYTEMSK(GOP_Reg_Base + REG_GOP_00, 0x0800) >> 11);

        // colorkey value
        *u8R = (u8)R2BYTEMSK(GOP_Reg_Base + REG_GOP_25, 0x00FF);
        *u8G = (u8)(R2BYTEMSK(GOP_Reg_Base + REG_GOP_24, 0xFF00) >> 8);
        *u8B = (u8)R2BYTEMSK(GOP_Reg_Base + REG_GOP_24, 0x00FF);
    }
    else
    {
        HALGOPERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }

    HALGOPDBG("[GOP]%s %d: Id:%02x, bEn:%d, (R=%x, G=%x, B=%x)\n", __FUNCTION__, __LINE__, eGopId, *bEn, *u8R, *u8G, *u8B);


}
