// $Change: 1008272 $
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012-2014 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("; MStar; Confidential; Information;") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

//****************************************************
// Drive Chip           : I2
// Excel CodeGen Version: 1.05
// Excel SW      Version: 1.01
// Excel update date    : 1/11/2018 13:41
//****************************************************

#ifndef _HAL_DAC_HDMITX_ATOP_TBL_C_
#define _HAL_DAC_HDMITX_ATOP_TBL_C_

#include "hal_dac_hdmitx_atop_tbl.h"

//****************************************************
// INIT_DAC_HDMITX_MISC
//****************************************************
MS_U8 MST_INIT_DAC_HDMITX_MISC_COMMON_TBL[MS_INIT_DAC_HDMITX_MISC_COMMON_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + REG_DATA_SIZE] =
{
 { DRV_REG(REG_DAC_HDMITX_MISC_1C_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_MISC_1C_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_MISC_1D_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_MISC_1D_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_MISC_1E_L), 0xFF, 0xFF,
 },
 { DRV_REG(REG_DAC_HDMITX_MISC_1E_H), 0xFF, 0xFF,
 },
 { DRV_REG(REG_DAC_HDMITX_MISC_1F_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_MISC_1F_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_MISC_52_L), 0x01, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_MISC_5D_H), 0x00, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_MISC_1B_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_MISC_1B_H), 0xFF, 0x00,
 },
};

//****************************************************
// INIT_DAC_HDMITX_PHY
//****************************************************
MS_U8 MST_INIT_DAC_HDMITX_PHY_COMMON_TBL[MS_INIT_DAC_HDMITX_PHY_COMMON_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + REG_DATA_SIZE] =
{
 { DRV_REG(REG_DAC_HDMITX_PHY_10_L), 0x01, 0x01,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_10_L), 0x10, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_10_H), 0x01, 0x01,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_10_H), 0x10, 0x10,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_10_L), 0x0c, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_10_H), 0x0c, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_10_H), 0xc0, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_11_L), 0x01, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_16_L), 0x03, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_16_L), 0x04, 0x04,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_17_L), 0x03, 0x01,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_17_L), 0x04, 0x04,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_18_L), 0x03, 0x02,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_18_L), 0x04, 0x04,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_19_L), 0x03, 0x03,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_19_L), 0x04, 0x04,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_15_H), 0x70, 0x30,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_02_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_02_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_03_L), 0xFF, 0x40,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_03_H), 0x0F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_79_H), 0x10, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_26_H), 0x01, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_46_H), 0x01, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_3D_H), 0x03, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_3D_L), 0x0F, 0x03,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_38_L), 0x03, 0x03,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_44_H), 0x30, 0x30,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_3C_L), 0x03, 0x01,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_3C_L), 0x78, 0x28,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_3C_H), 0x30, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_34_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_35_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_36_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_37_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_30_L), 0x3F, 0x14,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_30_H), 0x3F, 0x14,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_31_L), 0x3F, 0x14,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_31_H), 0x3F, 0x14,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_2E_H), 0x0A, 0x0A,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_2F_H), 0x07, 0x07,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_39_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_39_H), 0x0F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_3A_L), 0x70, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_3A_H), 0x0F, 0x0F,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_38_L), 0xF0, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_46_H), 0x40, 0x40,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_3C_H), 0x03, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_68_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_39_H), 0xF0, 0xF0,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_32_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_32_H), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_33_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_33_H), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_69_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_6A_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_6B_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_79_L), 0x01, 0x00,
 },
 { DRV_REG(REG_DAC_HDMITX_PHY_79_L), 0x02, 0x00,
 },
};

MS_DAC_ATOP_INFO stHAL_DAC_HDMITX_ATOP_TBL[MS_DAC_ATOP_TAB_NUM]=
{
    {*MST_INIT_DAC_HDMITX_MISC_COMMON_TBL,MS_INIT_DAC_HDMITX_MISC_COMMON_REG_NUM, MS_DAC_ATOP_IP_COMMON},
    {*MST_INIT_DAC_HDMITX_PHY_COMMON_TBL,MS_INIT_DAC_HDMITX_PHY_COMMON_REG_NUM, MS_DAC_ATOP_IP_COMMON},
};
#endif

