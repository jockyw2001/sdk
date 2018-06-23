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
// Excel update date    : 1/4/2018 18:05
//****************************************************

#ifndef _HAL_HDMITX_ATOP_TBL_C_
#define _HAL_HDMITX_ATOP_TBL_C_

#include "hal_hdmitx_atop_tbl.h"

//****************************************************
// INIT_HDMITX_MISC
//****************************************************
MS_U8 MST_INIT_HDMITX_MISC_COMMON_TBL[MS_INIT_HDMITX_MISC_COMMON_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + REG_DATA_SIZE] =
{
 { DRV_REG(REG_HDMITX_MISC_1C_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_MISC_1C_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_MISC_1D_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_MISC_1D_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_MISC_1E_L), 0xFF, 0xFF,
 },
 { DRV_REG(REG_HDMITX_MISC_1E_H), 0xFF, 0xFF,
 },
 { DRV_REG(REG_HDMITX_MISC_1F_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_MISC_1F_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_MISC_1B_H), 0xFF, 0x00,
 },
};

MS_U8 MST_INIT_HDMITX_MISC_TBL[MS_INIT_HDMITX_MISC_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + MS_TX_ATOP_ID_NUM]=
{
 { DRV_REG(REG_HDMITX_MISC_52_L), 0x01, 0x00, /*8bits_480_60I*/
                    0x00, /*8bits_480_60P*/
                    0x00, /*8bits_1080_60I*/
                    0x00, /*8bits_720_60P*/
                    0x00, /*8bits_1080_60P*/
                    0x00, /*8bits_4K2K_420_30P*/
                    0x00, /*8bits_4K2K_3D_30P*/
                    0x01, /*8bits_4K2K_420_60P*/
                    0x00, /*8bits_4K2K_60P*/
                    0x00, /*10bits_480_60I*/
                    0x00, /*10bits_480_60P*/
                    0x00, /*10bits_1080_60I*/
                    0x00, /*10bits_720_60P*/
                    0x00, /*10bits_1080_60P*/
                    0x00, /*10bits_4K2K_420_30P*/
                    0x00, /*10bits_4K2K_3D_30P*/
                    0x01, /*10bits_4K2K_420_60P*/
                    0x01, /*10bits_4K2K_60P_RB*/
                    0x00, /*12bits_480_60I*/
                    0x00, /*12bits_480_60P*/
                    0x00, /*12bits_1080_60I*/
                    0x00, /*12bits_720_60P*/
                    0x00, /*12bits_1080_60P*/
                    0x00, /*12bits_4K2K_420_30P*/
                    0x01, /*12bits_4K2K_3D_30P*/
                    0x01, /*12bits_4K2K_420_60P*/
                    0x00, /*16bits_480_60I*/
                    0x00, /*16bits_480_60P*/
                    0x00, /*16bits_1080_60I*/
                    0x00, /*16bits_720_60P*/
                    0x00, /*16bits_1080_60P*/
                    0x00, /*16bits_4K2K_420_30P*/
                    0x01, /*16bits_4K2K_3D_30P*/
                    0x01, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_MISC_5D_H), 0x00, 0x00, /*8bits_480_60I*/
                    0x00, /*8bits_480_60P*/
                    0x00, /*8bits_1080_60I*/
                    0x00, /*8bits_720_60P*/
                    0x00, /*8bits_1080_60P*/
                    0x00, /*8bits_4K2K_420_30P*/
                    0x00, /*8bits_4K2K_3D_30P*/
                    0x01, /*8bits_4K2K_420_60P*/
                    0x00, /*8bits_4K2K_60P*/
                    0x00, /*10bits_480_60I*/
                    0x00, /*10bits_480_60P*/
                    0x00, /*10bits_1080_60I*/
                    0x00, /*10bits_720_60P*/
                    0x00, /*10bits_1080_60P*/
                    0x00, /*10bits_4K2K_420_30P*/
                    0x00, /*10bits_4K2K_3D_30P*/
                    0x01, /*10bits_4K2K_420_60P*/
                    0x01, /*10bits_4K2K_60P_RB*/
                    0x00, /*12bits_480_60I*/
                    0x00, /*12bits_480_60P*/
                    0x00, /*12bits_1080_60I*/
                    0x00, /*12bits_720_60P*/
                    0x00, /*12bits_1080_60P*/
                    0x00, /*12bits_4K2K_420_30P*/
                    0x00, /*12bits_4K2K_3D_30P*/
                    0x00, /*12bits_4K2K_420_60P*/
                    0x00, /*16bits_480_60I*/
                    0x00, /*16bits_480_60P*/
                    0x00, /*16bits_1080_60I*/
                    0x00, /*16bits_720_60P*/
                    0x00, /*16bits_1080_60P*/
                    0x00, /*16bits_4K2K_420_30P*/
                    0x00, /*16bits_4K2K_3D_30P*/
                    0x00, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_MISC_1B_L), 0xFF, 0x40, /*8bits_480_60I*/
                    0x00, /*8bits_480_60P*/
                    0x40, /*8bits_1080_60I*/
                    0x00, /*8bits_720_60P*/
                    0x00, /*8bits_1080_60P*/
                    0x40, /*8bits_4K2K_420_30P*/
                    0x00, /*8bits_4K2K_3D_30P*/
                    0x00, /*8bits_4K2K_420_60P*/
                    0x40, /*8bits_4K2K_60P*/
                    0x40, /*10bits_480_60I*/
                    0x00, /*10bits_480_60P*/
                    0x40, /*10bits_1080_60I*/
                    0x00, /*10bits_720_60P*/
                    0x00, /*10bits_1080_60P*/
                    0x40, /*10bits_4K2K_420_30P*/
                    0x40, /*10bits_4K2K_3D_30P*/
                    0x00, /*10bits_4K2K_420_60P*/
                    0x00, /*10bits_4K2K_60P_RB*/
                    0x40, /*12bits_480_60I*/
                    0x00, /*12bits_480_60P*/
                    0x40, /*12bits_1080_60I*/
                    0x00, /*12bits_720_60P*/
                    0x00, /*12bits_1080_60P*/
                    0x40, /*12bits_4K2K_420_30P*/
                    0x00, /*12bits_4K2K_3D_30P*/
                    0x40, /*12bits_4K2K_420_60P*/
                    0x40, /*16bits_480_60I*/
                    0x00, /*16bits_480_60P*/
                    0x40, /*16bits_1080_60I*/
                    0x00, /*16bits_720_60P*/
                    0x00, /*16bits_1080_60P*/
                    0x40, /*16bits_4K2K_420_30P*/
                    0x00, /*16bits_4K2K_3D_30P*/
                    0x40, /*16bits_4K2K_420_60P*/
 },
};

//****************************************************
// INIT_HDMITX_PHY
//****************************************************
MS_U8 MST_INIT_HDMITX_PHY_COMMON_TBL[MS_INIT_HDMITX_PHY_COMMON_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + REG_DATA_SIZE] =
{
 { DRV_REG(REG_HDMITX_PHY_10_L), 0x01, 0x01,
 },
 { DRV_REG(REG_HDMITX_PHY_10_L), 0x10, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_10_H), 0x01, 0x01,
 },
 { DRV_REG(REG_HDMITX_PHY_10_H), 0x10, 0x10,
 },
 { DRV_REG(REG_HDMITX_PHY_10_L), 0x0c, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_10_H), 0x0c, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_10_H), 0xc0, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_11_L), 0x01, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_16_L), 0x03, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_16_L), 0x04, 0x04,
 },
 { DRV_REG(REG_HDMITX_PHY_17_L), 0x03, 0x01,
 },
 { DRV_REG(REG_HDMITX_PHY_17_L), 0x04, 0x04,
 },
 { DRV_REG(REG_HDMITX_PHY_18_L), 0x03, 0x02,
 },
 { DRV_REG(REG_HDMITX_PHY_18_L), 0x04, 0x04,
 },
 { DRV_REG(REG_HDMITX_PHY_19_L), 0x03, 0x03,
 },
 { DRV_REG(REG_HDMITX_PHY_19_L), 0x04, 0x04,
 },
 { DRV_REG(REG_HDMITX_PHY_03_H), 0x0F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_79_H), 0x10, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_26_H), 0x01, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_3D_H), 0x03, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_3D_L), 0x0F, 0x03,
 },
 { DRV_REG(REG_HDMITX_PHY_3C_L), 0x03, 0x01,
 },
 { DRV_REG(REG_HDMITX_PHY_3C_L), 0x78, 0x28,
 },
 { DRV_REG(REG_HDMITX_PHY_34_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_35_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_36_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_37_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_30_L), 0x3F, 0x14,
 },
 { DRV_REG(REG_HDMITX_PHY_2E_H), 0x0A, 0x0A,
 },
 { DRV_REG(REG_HDMITX_PHY_2F_H), 0x07, 0x07,
 },
 { DRV_REG(REG_HDMITX_PHY_39_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_39_H), 0x0F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_3A_L), 0x70, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_38_L), 0xF0, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_3C_H), 0x03, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_68_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_39_H), 0xF0, 0xF0,
 },
 { DRV_REG(REG_HDMITX_PHY_32_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_32_H), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_33_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_33_H), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_69_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_6A_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_6B_L), 0x3F, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_79_L), 0x01, 0x00,
 },
 { DRV_REG(REG_HDMITX_PHY_79_L), 0x02, 0x00,
 },
};

MS_U8 MST_INIT_HDMITX_PHY_TBL[MS_INIT_HDMITX_PHY_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + MS_TX_ATOP_ID_NUM]=
{
 { DRV_REG(REG_HDMITX_PHY_15_H), 0x70, 0x30, /*8bits_480_60I*/
                    0x30, /*8bits_480_60P*/
                    0x20, /*8bits_1080_60I*/
                    0x20, /*8bits_720_60P*/
                    0x10, /*8bits_1080_60P*/
                    0x01, /*8bits_4K2K_420_30P*/
                    0x10, /*8bits_4K2K_3D_30P*/
                    0x00, /*8bits_4K2K_420_60P*/
                    0x30, /*8bits_4K2K_60P*/
                    0x30, /*10bits_480_60I*/
                    0x30, /*10bits_480_60P*/
                    0x20, /*10bits_1080_60I*/
                    0x20, /*10bits_720_60P*/
                    0x10, /*10bits_1080_60P*/
                    0x10, /*10bits_4K2K_420_30P*/
                    0x00, /*10bits_4K2K_3D_30P*/
                    0x00, /*10bits_4K2K_420_60P*/
                    0x00, /*10bits_4K2K_60P_RB*/
                    0x30, /*12bits_480_60I*/
                    0x30, /*12bits_480_60P*/
                    0x20, /*12bits_1080_60I*/
                    0x20, /*12bits_720_60P*/
                    0x10, /*12bits_1080_60P*/
                    0x10, /*12bits_4K2K_420_30P*/
                    0x00, /*12bits_4K2K_3D_30P*/
                    0x00, /*12bits_4K2K_420_60P*/
                    0x20, /*16bits_480_60I*/
                    0x20, /*16bits_480_60P*/
                    0x10, /*16bits_1080_60I*/
                    0x10, /*16bits_720_60P*/
                    0x00, /*16bits_1080_60P*/
                    0x00, /*16bits_4K2K_420_30P*/
                    0x00, /*16bits_4K2K_3D_30P*/
                    0x00, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_02_L), 0xFF, 0x00, /*8bits_480_60I*/
                    0x00, /*8bits_480_60P*/
                    0xA2, /*8bits_1080_60I*/
                    0xA2, /*8bits_720_60P*/
                    0xA2, /*8bits_1080_60P*/
                    0xA2, /*8bits_4K2K_420_30P*/
                    0xD1, /*8bits_4K2K_3D_30P*/
                    0xD1, /*8bits_4K2K_420_60P*/
                    0x00, /*8bits_4K2K_60P*/
                    0x33, /*10bits_480_60I*/
                    0x33, /*10bits_480_60P*/
                    0x82, /*10bits_1080_60I*/
                    0x82, /*10bits_720_60P*/
                    0x82, /*10bits_1080_60P*/
                    0x82, /*10bits_4K2K_420_30P*/
                    0x82, /*10bits_4K2K_3D_30P*/
                    0x82, /*10bits_4K2K_420_60P*/
                    0x09, /*10bits_4K2K_60P_RB*/
                    0xAA, /*12bits_480_60I*/
                    0xAA, /*12bits_480_60P*/
                    0xC1, /*12bits_1080_60I*/
                    0xC1, /*12bits_720_60P*/
                    0xC1, /*12bits_1080_60P*/
                    0xC1, /*12bits_4K2K_420_30P*/
                    0xC1, /*12bits_4K2K_3D_30P*/
                    0xC1, /*12bits_4K2K_420_60P*/
                    0x00, /*16bits_480_60I*/
                    0x00, /*16bits_480_60P*/
                    0xA2, /*16bits_1080_60I*/
                    0xA2, /*16bits_720_60P*/
                    0xA2, /*16bits_1080_60P*/
                    0xA2, /*16bits_4K2K_420_30P*/
                    0xD1, /*16bits_4K2K_3D_30P*/
                    0xD1, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_02_H), 0xFF, 0x00, /*8bits_480_60I*/
                    0x00, /*8bits_480_60P*/
                    0x8B, /*8bits_1080_60I*/
                    0x8B, /*8bits_720_60P*/
                    0x8B, /*8bits_1080_60P*/
                    0x8B, /*8bits_4K2K_420_30P*/
                    0x45, /*8bits_4K2K_3D_30P*/
                    0x45, /*8bits_4K2K_420_60P*/
                    0x00, /*8bits_4K2K_60P*/
                    0x33, /*10bits_480_60I*/
                    0x33, /*10bits_480_60P*/
                    0x3C, /*10bits_1080_60I*/
                    0x3C, /*10bits_720_60P*/
                    0x3C, /*10bits_1080_60P*/
                    0x3C, /*10bits_4K2K_420_30P*/
                    0x3C, /*10bits_4K2K_3D_30P*/
                    0x3C, /*10bits_4K2K_420_60P*/
                    0xF2, /*10bits_4K2K_60P_RB*/
                    0xAA, /*12bits_480_60I*/
                    0xAA, /*12bits_480_60P*/
                    0x07, /*12bits_1080_60I*/
                    0x07, /*12bits_720_60P*/
                    0x07, /*12bits_1080_60P*/
                    0x07, /*12bits_4K2K_420_30P*/
                    0x07, /*12bits_4K2K_3D_30P*/
                    0x07, /*12bits_4K2K_420_60P*/
                    0x00, /*16bits_480_60I*/
                    0x00, /*16bits_480_60P*/
                    0x8B, /*16bits_1080_60I*/
                    0x8B, /*16bits_720_60P*/
                    0x8B, /*16bits_1080_60P*/
                    0x8B, /*16bits_4K2K_420_30P*/
                    0x45, /*16bits_4K2K_3D_30P*/
                    0x45, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_03_L), 0xFF, 0x40, /*8bits_480_60I*/
                    0x40, /*8bits_480_60P*/
                    0x2E, /*8bits_1080_60I*/
                    0x2E, /*8bits_720_60P*/
                    0x2E, /*8bits_1080_60P*/
                    0x2E, /*8bits_4K2K_420_30P*/
                    0x17, /*8bits_4K2K_3D_30P*/
                    0x17, /*8bits_4K2K_420_60P*/
                    0x40, /*8bits_4K2K_60P*/
                    0x33, /*10bits_480_60I*/
                    0x33, /*10bits_480_60P*/
                    0x25, /*10bits_1080_60I*/
                    0x25, /*10bits_720_60P*/
                    0x25, /*10bits_1080_60P*/
                    0x25, /*10bits_4K2K_420_30P*/
                    0x25, /*10bits_4K2K_3D_30P*/
                    0x25, /*10bits_4K2K_420_60P*/
                    0x14, /*10bits_4K2K_60P_RB*/
                    0x2A, /*12bits_480_60I*/
                    0x2A, /*12bits_480_60P*/
                    0x1F, /*12bits_1080_60I*/
                    0x1F, /*12bits_720_60P*/
                    0x1F, /*12bits_1080_60P*/
                    0x1F, /*12bits_4K2K_420_30P*/
                    0x1F, /*12bits_4K2K_3D_30P*/
                    0x1F, /*12bits_4K2K_420_60P*/
                    0x40, /*16bits_480_60I*/
                    0x40, /*16bits_480_60P*/
                    0x2E, /*16bits_1080_60I*/
                    0x2E, /*16bits_720_60P*/
                    0x2E, /*16bits_1080_60P*/
                    0x2E, /*16bits_4K2K_420_30P*/
                    0x17, /*16bits_4K2K_3D_30P*/
                    0x17, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_46_H), 0x01, 0x00, /*8bits_480_60I*/
                    0x00, /*8bits_480_60P*/
                    0x00, /*8bits_1080_60I*/
                    0x00, /*8bits_720_60P*/
                    0x00, /*8bits_1080_60P*/
                    0x00, /*8bits_4K2K_420_30P*/
                    0x00, /*8bits_4K2K_3D_30P*/
                    0x01, /*8bits_4K2K_420_60P*/
                    0x00, /*8bits_4K2K_60P*/
                    0x00, /*10bits_480_60I*/
                    0x00, /*10bits_480_60P*/
                    0x00, /*10bits_1080_60I*/
                    0x00, /*10bits_720_60P*/
                    0x00, /*10bits_1080_60P*/
                    0x00, /*10bits_4K2K_420_30P*/
                    0x01, /*10bits_4K2K_3D_30P*/
                    0x01, /*10bits_4K2K_420_60P*/
                    0x01, /*10bits_4K2K_60P_RB*/
                    0x00, /*12bits_480_60I*/
                    0x00, /*12bits_480_60P*/
                    0x00, /*12bits_1080_60I*/
                    0x00, /*12bits_720_60P*/
                    0x00, /*12bits_1080_60P*/
                    0x00, /*12bits_4K2K_420_30P*/
                    0x01, /*12bits_4K2K_3D_30P*/
                    0x01, /*12bits_4K2K_420_60P*/
                    0x00, /*16bits_480_60I*/
                    0x00, /*16bits_480_60P*/
                    0x00, /*16bits_1080_60I*/
                    0x00, /*16bits_720_60P*/
                    0x00, /*16bits_1080_60P*/
                    0x00, /*16bits_4K2K_420_30P*/
                    0x01, /*16bits_4K2K_3D_30P*/
                    0x01, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_38_L), 0x03, 0x03, /*8bits_480_60I*/
                    0x03, /*8bits_480_60P*/
                    0x02, /*8bits_1080_60I*/
                    0x02, /*8bits_720_60P*/
                    0x01, /*8bits_1080_60P*/
                    0x01, /*8bits_4K2K_420_30P*/
                    0x01, /*8bits_4K2K_3D_30P*/
                    0x00, /*8bits_4K2K_420_60P*/
                    0x03, /*8bits_4K2K_60P*/
                    0x03, /*10bits_480_60I*/
                    0x03, /*10bits_480_60P*/
                    0x02, /*10bits_1080_60I*/
                    0x02, /*10bits_720_60P*/
                    0x01, /*10bits_1080_60P*/
                    0x01, /*10bits_4K2K_420_30P*/
                    0x00, /*10bits_4K2K_3D_30P*/
                    0x00, /*10bits_4K2K_420_60P*/
                    0x00, /*10bits_4K2K_60P_RB*/
                    0x03, /*12bits_480_60I*/
                    0x03, /*12bits_480_60P*/
                    0x02, /*12bits_1080_60I*/
                    0x02, /*12bits_720_60P*/
                    0x01, /*12bits_1080_60P*/
                    0x01, /*12bits_4K2K_420_30P*/
                    0x00, /*12bits_4K2K_3D_30P*/
                    0x00, /*12bits_4K2K_420_60P*/
                    0x02, /*16bits_480_60I*/
                    0x02, /*16bits_480_60P*/
                    0x01, /*16bits_1080_60I*/
                    0x01, /*16bits_720_60P*/
                    0x00, /*16bits_1080_60P*/
                    0x00, /*16bits_4K2K_420_30P*/
                    0x00, /*16bits_4K2K_3D_30P*/
                    0x00, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_44_H), 0x30, 0x30, /*8bits_480_60I*/
                    0x30, /*8bits_480_60P*/
                    0x20, /*8bits_1080_60I*/
                    0x20, /*8bits_720_60P*/
                    0x10, /*8bits_1080_60P*/
                    0x10, /*8bits_4K2K_420_30P*/
                    0x10, /*8bits_4K2K_3D_30P*/
                    0x00, /*8bits_4K2K_420_60P*/
                    0x30, /*8bits_4K2K_60P*/
                    0x30, /*10bits_480_60I*/
                    0x30, /*10bits_480_60P*/
                    0x20, /*10bits_1080_60I*/
                    0x20, /*10bits_720_60P*/
                    0x10, /*10bits_1080_60P*/
                    0x10, /*10bits_4K2K_420_30P*/
                    0x00, /*10bits_4K2K_3D_30P*/
                    0x00, /*10bits_4K2K_420_60P*/
                    0x00, /*10bits_4K2K_60P_RB*/
                    0x30, /*12bits_480_60I*/
                    0x30, /*12bits_480_60P*/
                    0x20, /*12bits_1080_60I*/
                    0x20, /*12bits_720_60P*/
                    0x10, /*12bits_1080_60P*/
                    0x10, /*12bits_4K2K_420_30P*/
                    0x00, /*12bits_4K2K_3D_30P*/
                    0x00, /*12bits_4K2K_420_60P*/
                    0x20, /*16bits_480_60I*/
                    0x20, /*16bits_480_60P*/
                    0x10, /*16bits_1080_60I*/
                    0x10, /*16bits_720_60P*/
                    0x00, /*16bits_1080_60P*/
                    0x00, /*16bits_4K2K_420_30P*/
                    0x00, /*16bits_4K2K_3D_30P*/
                    0x00, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_3C_H), 0x30, 0x00, /*8bits_480_60I*/
                    0x00, /*8bits_480_60P*/
                    0x00, /*8bits_1080_60I*/
                    0x00, /*8bits_720_60P*/
                    0x00, /*8bits_1080_60P*/
                    0x00, /*8bits_4K2K_420_30P*/
                    0x00, /*8bits_4K2K_3D_30P*/
                    0x00, /*8bits_4K2K_420_60P*/
                    0x00, /*8bits_4K2K_60P*/
                    0x10, /*10bits_480_60I*/
                    0x10, /*10bits_480_60P*/
                    0x10, /*10bits_1080_60I*/
                    0x10, /*10bits_720_60P*/
                    0x10, /*10bits_1080_60P*/
                    0x10, /*10bits_4K2K_420_30P*/
                    0x10, /*10bits_4K2K_3D_30P*/
                    0x10, /*10bits_4K2K_420_60P*/
                    0x10, /*10bits_4K2K_60P_RB*/
                    0x20, /*12bits_480_60I*/
                    0x20, /*12bits_480_60P*/
                    0x20, /*12bits_1080_60I*/
                    0x20, /*12bits_720_60P*/
                    0x20, /*12bits_1080_60P*/
                    0x20, /*12bits_4K2K_420_30P*/
                    0x20, /*12bits_4K2K_3D_30P*/
                    0x20, /*12bits_4K2K_420_60P*/
                    0x30, /*16bits_480_60I*/
                    0x30, /*16bits_480_60P*/
                    0x30, /*16bits_1080_60I*/
                    0x30, /*16bits_720_60P*/
                    0x30, /*16bits_1080_60P*/
                    0x30, /*16bits_4K2K_420_30P*/
                    0x30, /*16bits_4K2K_3D_30P*/
                    0x30, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_30_H), 0x3F, 0x14, /*8bits_480_60I*/
                    0x14, /*8bits_480_60P*/
                    0x14, /*8bits_1080_60I*/
                    0x14, /*8bits_720_60P*/
                    0x14, /*8bits_1080_60P*/
                    0x14, /*8bits_4K2K_420_30P*/
                    0x38, /*8bits_4K2K_3D_30P*/
                    0x38, /*8bits_4K2K_420_60P*/
                    0x14, /*8bits_4K2K_60P*/
                    0x14, /*10bits_480_60I*/
                    0x14, /*10bits_480_60P*/
                    0x14, /*10bits_1080_60I*/
                    0x14, /*10bits_720_60P*/
                    0x14, /*10bits_1080_60P*/
                    0x14, /*10bits_4K2K_420_30P*/
                    0x38, /*10bits_4K2K_3D_30P*/
                    0x38, /*10bits_4K2K_420_60P*/
                    0x38, /*10bits_4K2K_60P_RB*/
                    0x14, /*12bits_480_60I*/
                    0x14, /*12bits_480_60P*/
                    0x14, /*12bits_1080_60I*/
                    0x14, /*12bits_720_60P*/
                    0x38, /*12bits_1080_60P*/
                    0x38, /*12bits_4K2K_420_30P*/
                    0x38, /*12bits_4K2K_3D_30P*/
                    0x38, /*12bits_4K2K_420_60P*/
                    0x14, /*16bits_480_60I*/
                    0x14, /*16bits_480_60P*/
                    0x14, /*16bits_1080_60I*/
                    0x14, /*16bits_720_60P*/
                    0x38, /*16bits_1080_60P*/
                    0x38, /*16bits_4K2K_420_30P*/
                    0x38, /*16bits_4K2K_3D_30P*/
                    0x38, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_31_L), 0x3F, 0x14, /*8bits_480_60I*/
                    0x14, /*8bits_480_60P*/
                    0x14, /*8bits_1080_60I*/
                    0x14, /*8bits_720_60P*/
                    0x14, /*8bits_1080_60P*/
                    0x14, /*8bits_4K2K_420_30P*/
                    0x38, /*8bits_4K2K_3D_30P*/
                    0x38, /*8bits_4K2K_420_60P*/
                    0x14, /*8bits_4K2K_60P*/
                    0x14, /*10bits_480_60I*/
                    0x14, /*10bits_480_60P*/
                    0x14, /*10bits_1080_60I*/
                    0x14, /*10bits_720_60P*/
                    0x14, /*10bits_1080_60P*/
                    0x14, /*10bits_4K2K_420_30P*/
                    0x38, /*10bits_4K2K_3D_30P*/
                    0x38, /*10bits_4K2K_420_60P*/
                    0x38, /*10bits_4K2K_60P_RB*/
                    0x14, /*12bits_480_60I*/
                    0x14, /*12bits_480_60P*/
                    0x14, /*12bits_1080_60I*/
                    0x14, /*12bits_720_60P*/
                    0x38, /*12bits_1080_60P*/
                    0x38, /*12bits_4K2K_420_30P*/
                    0x38, /*12bits_4K2K_3D_30P*/
                    0x38, /*12bits_4K2K_420_60P*/
                    0x14, /*16bits_480_60I*/
                    0x14, /*16bits_480_60P*/
                    0x14, /*16bits_1080_60I*/
                    0x14, /*16bits_720_60P*/
                    0x38, /*16bits_1080_60P*/
                    0x38, /*16bits_4K2K_420_30P*/
                    0x38, /*16bits_4K2K_3D_30P*/
                    0x38, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_31_H), 0x3F, 0x14, /*8bits_480_60I*/
                    0x14, /*8bits_480_60P*/
                    0x14, /*8bits_1080_60I*/
                    0x14, /*8bits_720_60P*/
                    0x14, /*8bits_1080_60P*/
                    0x14, /*8bits_4K2K_420_30P*/
                    0x38, /*8bits_4K2K_3D_30P*/
                    0x38, /*8bits_4K2K_420_60P*/
                    0x14, /*8bits_4K2K_60P*/
                    0x14, /*10bits_480_60I*/
                    0x14, /*10bits_480_60P*/
                    0x14, /*10bits_1080_60I*/
                    0x14, /*10bits_720_60P*/
                    0x14, /*10bits_1080_60P*/
                    0x14, /*10bits_4K2K_420_30P*/
                    0x38, /*10bits_4K2K_3D_30P*/
                    0x38, /*10bits_4K2K_420_60P*/
                    0x38, /*10bits_4K2K_60P_RB*/
                    0x14, /*12bits_480_60I*/
                    0x14, /*12bits_480_60P*/
                    0x14, /*12bits_1080_60I*/
                    0x14, /*12bits_720_60P*/
                    0x38, /*12bits_1080_60P*/
                    0x38, /*12bits_4K2K_420_30P*/
                    0x38, /*12bits_4K2K_3D_30P*/
                    0x38, /*12bits_4K2K_420_60P*/
                    0x14, /*16bits_480_60I*/
                    0x14, /*16bits_480_60P*/
                    0x14, /*16bits_1080_60I*/
                    0x14, /*16bits_720_60P*/
                    0x38, /*16bits_1080_60P*/
                    0x38, /*16bits_4K2K_420_30P*/
                    0x38, /*16bits_4K2K_3D_30P*/
                    0x38, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_3A_H), 0x0F, 0x0F, /*8bits_480_60I*/
                    0x0F, /*8bits_480_60P*/
                    0x0F, /*8bits_1080_60I*/
                    0x0F, /*8bits_720_60P*/
                    0x0F, /*8bits_1080_60P*/
                    0x0F, /*8bits_4K2K_420_30P*/
                    0x08, /*8bits_4K2K_3D_30P*/
                    0x08, /*8bits_4K2K_420_60P*/
                    0x0F, /*8bits_4K2K_60P*/
                    0x0F, /*10bits_480_60I*/
                    0x0F, /*10bits_480_60P*/
                    0x0F, /*10bits_1080_60I*/
                    0x0F, /*10bits_720_60P*/
                    0x08, /*10bits_1080_60P*/
                    0x08, /*10bits_4K2K_420_30P*/
                    0x08, /*10bits_4K2K_3D_30P*/
                    0x08, /*10bits_4K2K_420_60P*/
                    0x08, /*10bits_4K2K_60P_RB*/
                    0x0F, /*12bits_480_60I*/
                    0x0F, /*12bits_480_60P*/
                    0x0F, /*12bits_1080_60I*/
                    0x0F, /*12bits_720_60P*/
                    0x08, /*12bits_1080_60P*/
                    0x08, /*12bits_4K2K_420_30P*/
                    0x08, /*12bits_4K2K_3D_30P*/
                    0x08, /*12bits_4K2K_420_60P*/
                    0x0F, /*16bits_480_60I*/
                    0x0F, /*16bits_480_60P*/
                    0x0F, /*16bits_1080_60I*/
                    0x0F, /*16bits_720_60P*/
                    0x08, /*16bits_1080_60P*/
                    0x08, /*16bits_4K2K_420_30P*/
                    0x08, /*16bits_4K2K_3D_30P*/
                    0x08, /*16bits_4K2K_420_60P*/
 },
 { DRV_REG(REG_HDMITX_PHY_46_H), 0x40, 0x40, /*8bits_480_60I*/
                    0x40, /*8bits_480_60P*/
                    0x40, /*8bits_1080_60I*/
                    0x40, /*8bits_720_60P*/
                    0x00, /*8bits_1080_60P*/
                    0x00, /*8bits_4K2K_420_30P*/
                    0x40, /*8bits_4K2K_3D_30P*/
                    0x00, /*8bits_4K2K_420_60P*/
                    0x40, /*8bits_4K2K_60P*/
                    0x40, /*10bits_480_60I*/
                    0x40, /*10bits_480_60P*/
                    0x40, /*10bits_1080_60I*/
                    0x40, /*10bits_720_60P*/
                    0x00, /*10bits_1080_60P*/
                    0x00, /*10bits_4K2K_420_30P*/
                    0x40, /*10bits_4K2K_3D_30P*/
                    0x00, /*10bits_4K2K_420_60P*/
                    0x00, /*10bits_4K2K_60P_RB*/
                    0x40, /*12bits_480_60I*/
                    0x40, /*12bits_480_60P*/
                    0x40, /*12bits_1080_60I*/
                    0x40, /*12bits_720_60P*/
                    0x00, /*12bits_1080_60P*/
                    0x00, /*12bits_4K2K_420_30P*/
                    0x40, /*12bits_4K2K_3D_30P*/
                    0x40, /*12bits_4K2K_420_60P*/
                    0x40, /*16bits_480_60I*/
                    0x40, /*16bits_480_60P*/
                    0x40, /*16bits_1080_60I*/
                    0x40, /*16bits_720_60P*/
                    0x00, /*16bits_1080_60P*/
                    0x00, /*16bits_4K2K_420_30P*/
                    0x40, /*16bits_4K2K_3D_30P*/
                    0x40, /*16bits_4K2K_420_60P*/
 },
};

MS_TX_ATOP_INFO stHAL_HDMITX_ATOP_TBL[MS_TX_ATOP_TAB_NUM]=
{
    {*MST_INIT_HDMITX_MISC_TBL,MS_INIT_HDMITX_MISC_REG_NUM, MS_TX_ATOP_IP_NORMAL},
    {*MST_INIT_HDMITX_MISC_COMMON_TBL,MS_INIT_HDMITX_MISC_COMMON_REG_NUM, MS_TX_ATOP_IP_COMMON},
    {*MST_INIT_HDMITX_PHY_TBL,MS_INIT_HDMITX_PHY_REG_NUM, MS_TX_ATOP_IP_NORMAL},
    {*MST_INIT_HDMITX_PHY_COMMON_TBL,MS_INIT_HDMITX_PHY_COMMON_REG_NUM, MS_TX_ATOP_IP_COMMON},
};
#endif

