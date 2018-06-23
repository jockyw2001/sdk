// Generate Time: 2017-12-26 20:42:28.321794
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2016 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (MStar Confidential Information) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef __INFINITY2_REG_CLKGEN2__
#define __INFINITY2_REG_CLKGEN2__
typedef struct {
	// h0000, bit: 4
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_vp9 (0)
	#define mask_of_clkgen2_reg_ckg_vp9 (0x1f)
	unsigned int reg_ckg_vp9:5;

	// h0000, bit: 14
	/* */
	unsigned int :11;

	// h0000
	unsigned int /* padding 16 bit */:16;

	// h0001, bit: 4
	/* CLK_ECC clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	     000: XTAL
	     001: 54  MHz
	     010: 108   MHz
	     011: 160   MHz
	     100: 216   MHz
	     101: Reserved
	     110: Reserved
	     111: Reserved*/
	#define offset_of_clkgen2_reg_ckg_ecc (2)
	#define mask_of_clkgen2_reg_ckg_ecc (0x1f)
	unsigned int reg_ckg_ecc:5;

	// h0001, bit: 7
	/* */
	unsigned int :3;

	// h0001, bit: 12
	/* clk_gmac_ahb clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	     000:  144MHz
	     001:  172MHz
	     010:  192Mhz
	     011:  216MHz
	     100:  N/A
	     101:  N/A
	     110:  N/A
	     111:  N/A*/
	#define offset_of_clkgen2_reg_ckg_gmac_ahb (2)
	#define mask_of_clkgen2_reg_ckg_gmac_ahb (0x1f00)
	unsigned int reg_ckg_gmac_ahb:5;

	// h0001, bit: 14
	/* */
	unsigned int :3;

	// h0001
	unsigned int /* padding 16 bit */:16;

	// h0002, bit: 5
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_rasp0 (4)
	#define mask_of_clkgen2_reg_ckg_rasp0 (0x3f)
	unsigned int reg_ckg_rasp0:6;

	// h0002, bit: 14
	/* */
	unsigned int :10;

	// h0002
	unsigned int /* padding 16 bit */:16;

	// h0003, bit: 5
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_rasp1 (6)
	#define mask_of_clkgen2_reg_ckg_rasp1 (0x3f)
	unsigned int reg_ckg_rasp1:6;

	// h0003, bit: 14
	/* */
	unsigned int :10;

	// h0003
	unsigned int /* padding 16 bit */:16;

	// h0004, bit: 5
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_emm0 (8)
	#define mask_of_clkgen2_reg_ckg_emm0 (0x3f)
	unsigned int reg_ckg_emm0:6;

	// h0004, bit: 14
	/* */
	unsigned int :10;

	// h0004
	unsigned int /* padding 16 bit */:16;

	// h0005, bit: 5
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_emm1 (10)
	#define mask_of_clkgen2_reg_ckg_emm1 (0x3f)
	unsigned int reg_ckg_emm1:6;

	// h0005, bit: 14
	/* */
	unsigned int :10;

	// h0005
	unsigned int /* padding 16 bit */:16;

	// h0006, bit: 4
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_tso1_in (12)
	#define mask_of_clkgen2_reg_ckg_tso1_in (0x1f)
	unsigned int reg_ckg_tso1_in:5;

	// h0006, bit: 14
	/* */
	unsigned int :11;

	// h0006
	unsigned int /* padding 16 bit */:16;

	// h0007, bit: 4
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_tso2_in (14)
	#define mask_of_clkgen2_reg_ckg_tso2_in (0x1f)
	unsigned int reg_ckg_tso2_in:5;

	// h0007, bit: 14
	/* */
	unsigned int :11;

	// h0007
	unsigned int /* padding 16 bit */:16;

	// h0008, bit: 4
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_tso3_in (16)
	#define mask_of_clkgen2_reg_ckg_tso3_in (0x1f)
	unsigned int reg_ckg_tso3_in:5;

	// h0008, bit: 14
	/* */
	unsigned int :11;

	// h0008
	unsigned int /* padding 16 bit */:16;

	// h0009, bit: 4
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_tso4_in (18)
	#define mask_of_clkgen2_reg_ckg_tso4_in (0x1f)
	unsigned int reg_ckg_tso4_in:5;

	// h0009, bit: 14
	/* */
	unsigned int :11;

	// h0009
	unsigned int /* padding 16 bit */:16;

	// h000a, bit: 4
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_tso5_in (20)
	#define mask_of_clkgen2_reg_ckg_tso5_in (0x1f)
	unsigned int reg_ckg_tso5_in:5;

	// h000a, bit: 14
	/* */
	unsigned int :11;

	// h000a
	unsigned int /* padding 16 bit */:16;

	// h000b, bit: 14
	/* */
	unsigned int :16;

	// h000b
	unsigned int /* padding 16 bit */:16;

	// h000c, bit: 14
	/* */
	unsigned int :16;

	// h000c
	unsigned int /* padding 16 bit */:16;

	// h000d, bit: 14
	/* */
	unsigned int :16;

	// h000d
	unsigned int /* padding 16 bit */:16;

	// h000e, bit: 14
	/* */
	unsigned int :16;

	// h000e
	unsigned int /* padding 16 bit */:16;

	// h000f, bit: 14
	/* */
	unsigned int :16;

	// h000f
	unsigned int /* padding 16 bit */:16;

	// h0010, bit: 4
	/* CLK_MFE clock setting
	[0]:  disable clock
	[1]:  invert clock
	[3:2]:  Select clock source
	 000:600MHz
	 001:480MHz
	 002:432MHz
	 003:384MHz
	 004:288MHz
	 005:192MHz
	 006: 576Mhz
	  otherwise : reserved*/
	#define offset_of_clkgen2_reg_ckg_mfe (32)
	#define mask_of_clkgen2_reg_ckg_mfe (0x1f)
	unsigned int reg_ckg_mfe:5;

	// h0010, bit: 7
	/* */
	unsigned int :3;

	// h0010, bit: 12
	/* CLK_MFE clock setting
	[0]:  disable clock
	[1]:  invert clock
	[3:2]:  Select clock source
	 000:600MHz
	 001:480MHz
	 002:432MHz
	 003:384MHz
	 004:288MHz
	 005:192MHz
	 006: 576Mhz
	  otherwise : reserved*/
	#define offset_of_clkgen2_reg_ckg_mhe (32)
	#define mask_of_clkgen2_reg_ckg_mhe (0x1f00)
	unsigned int reg_ckg_mhe:5;

	// h0010, bit: 14
	/* */
	unsigned int :3;

	// h0010
	unsigned int /* padding 16 bit */:16;

	// h0011, bit: 3
	/* CLK_NJPD clock setting
	[0]: disable clock
	[1]: invert clock
	[3:2]: Select clock source
	     00: 288 MHz
	     01: 432 MHz
	     10: 192 MHz
	     11: 384 MHz*/
	#define offset_of_clkgen2_reg_ckg_njpd (34)
	#define mask_of_clkgen2_reg_ckg_njpd (0xf)
	unsigned int reg_ckg_njpd:4;

	// h0011, bit: 14
	/* */
	unsigned int :12;

	// h0011
	unsigned int /* padding 16 bit */:16;

	// h0012, bit: 4
	/* clk_smart clock settings
	[0] : disable clock
	[1] : invert clock
	[4:2] : Select clock source
	  000 : 172 MHz
	  001 : 160 MHz
	  010 : 144 MHz
	  011 : 123 MHz
	  100 : 108 MHz
	  otherwise : reserved*/
	#define offset_of_clkgen2_reg_ckg_smart (36)
	#define mask_of_clkgen2_reg_ckg_smart (0x1f)
	unsigned int reg_ckg_smart:5;

	// h0012, bit: 7
	/* */
	unsigned int :3;

	// h0012, bit: 12
	/* clk_smart clock settings
	[0] : disable clock
	[1] : invert clock
	[4:2] : Select clock source
	  000 : 172 MHz
	  001 : 160 MHz
	  010 : 144 MHz
	  011 : 123 MHz
	  100 : 108 MHz
	  otherwise : reserved*/
	#define offset_of_clkgen2_reg_ckg_smart_2 (36)
	#define mask_of_clkgen2_reg_ckg_smart_2 (0x1f00)
	unsigned int reg_ckg_smart_2:5;

	// h0012, bit: 14
	/* */
	unsigned int :3;

	// h0012
	unsigned int /* padding 16 bit */:16;

	// h0013, bit: 3
	/* [0] gate
	[1] inv
	[3:2]
	0: 160MHz
	1: 144MHZ
	2: 123MHz
	3: 72MHz*/
	#define offset_of_clkgen2_reg_ckg_zdec_vld (38)
	#define mask_of_clkgen2_reg_ckg_zdec_vld (0xf)
	unsigned int reg_ckg_zdec_vld:4;

	// h0013, bit: 14
	/* */
	unsigned int :12;

	// h0013
	unsigned int /* padding 16 bit */:16;

	// h0014, bit: 3
	/* [0] gate
	[1] inv
	[3:2]
	0: 192MHz
	1: 160MHZ
	2: 123MHz
	3: 72MHz*/
	#define offset_of_clkgen2_reg_ckg_zdec_lzd (40)
	#define mask_of_clkgen2_reg_ckg_zdec_lzd (0xf)
	unsigned int reg_ckg_zdec_lzd:4;

	// h0014, bit: 14
	/* */
	unsigned int :12;

	// h0014
	unsigned int /* padding 16 bit */:16;

	// h0015, bit: 7
	/* */
	#define offset_of_clkgen2_reg_mspi0_div (42)
	#define mask_of_clkgen2_reg_mspi0_div (0xff)
	unsigned int reg_mspi0_div:8;

	// h0015, bit: 14
	/* */
	unsigned int :8;

	// h0015
	unsigned int /* padding 16 bit */:16;

	// h0016, bit: 7
	/* */
	#define offset_of_clkgen2_reg_mspi1_div (44)
	#define mask_of_clkgen2_reg_mspi1_div (0xff)
	unsigned int reg_mspi1_div:8;

	// h0016, bit: 14
	/* */
	#define offset_of_clkgen2_reg_mspi2_div (44)
	#define mask_of_clkgen2_reg_mspi2_div (0xff00)
	unsigned int reg_mspi2_div:8;

	// h0016
	unsigned int /* padding 16 bit */:16;

	// h0017, bit: 4
	/* clk_evd clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	000: 384Mhz
	001: 320Mhz
	010: 240Mhz
	011: 192Mhz
	1xx: reserved*/
	#define offset_of_clkgen2_reg_ckg_evd_lite (46)
	#define mask_of_clkgen2_reg_ckg_evd_lite (0x1f)
	unsigned int reg_ckg_evd_lite:5;

	// h0017, bit: 7
	/* */
	unsigned int :3;

	// h0017, bit: 12
	/* CLK_EVD_PPU clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	   000:   345 MHz
	   001:   320 MHz
	   010:   288 MHz
	   011:   240 MHz
	   100:   216 MHz
	   101:   192 MHz
	   110:   160 MHz*/
	#define offset_of_clkgen2_reg_ckg_evd_ppu_lite (46)
	#define mask_of_clkgen2_reg_ckg_evd_ppu_lite (0x1f00)
	unsigned int reg_ckg_evd_ppu_lite:5;

	// h0017, bit: 14
	/* */
	unsigned int :3;

	// h0017
	unsigned int /* padding 16 bit */:16;

	// h0018, bit: 4
	/* CLK_HVD_AEC clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	000: 288MHz
	001: 240MHz
	010: 216MHz
	011:172MHz
	100:144MHz
	*/
	#define offset_of_clkgen2_reg_ckg_hvd_aec_lite (48)
	#define mask_of_clkgen2_reg_ckg_hvd_aec_lite (0x1f)
	unsigned int reg_ckg_hvd_aec_lite:5;

	// h0018, bit: 7
	/* */
	unsigned int :3;

	// h0018, bit: 12
	/* clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_hi_codec_lite (48)
	#define mask_of_clkgen2_reg_ckg_bist_hi_codec_lite (0x1f00)
	unsigned int reg_ckg_bist_hi_codec_lite:5;

	// h0018, bit: 14
	/* */
	unsigned int :3;

	// h0018
	unsigned int /* padding 16 bit */:16;

	// h0019, bit: 6
	/* clk_vd_mheg5 clock setting
	no use
	N/A*/
	#define offset_of_clkgen2_reg_ckg_vd_mheg5_lite (50)
	#define mask_of_clkgen2_reg_ckg_vd_mheg5_lite (0x7f)
	unsigned int reg_ckg_vd_mheg5_lite:7;

	// h0019, bit: 14
	/* */
	unsigned int :9;

	// h0019
	unsigned int /* padding 16 bit */:16;

	// h001a, bit: 2
	/* CLK_HV_IDB clock setting
	disable : reg_ckg_hvd[0],
	inverse : reg_ckg_hvd[1]
	[2:0] : Select clock source
	000: 432 MHz
	001 :384 MHz
	010: N/A
	011: 480 MHz
	100: 600 MHz
	others N/A
	*/
	#define offset_of_clkgen2_reg_ckg_hvd_idb (52)
	#define mask_of_clkgen2_reg_ckg_hvd_idb (0x7)
	unsigned int reg_ckg_hvd_idb:3;

	// h001a, bit: 14
	/* */
	unsigned int :13;

	// h001a
	unsigned int /* padding 16 bit */:16;

	// h001b, bit: 4
	/* CLK_HVD_AEC clock setting
	[0]: disable clock
	[1]: invert clock
	[3:2]: Select clock source
	00: 288MHz
	01: 240MHz
	10: 216MHz
	11: 320MHz

	*/
	#define offset_of_clkgen2_reg_ckg_hvd_aec (54)
	#define mask_of_clkgen2_reg_ckg_hvd_aec (0x1f)
	unsigned int reg_ckg_hvd_aec:5;

	// h001b, bit: 14
	/* */
	unsigned int :11;

	// h001b
	unsigned int /* padding 16 bit */:16;

	// h001c, bit: 4
	/* CLK_EVD_PPU clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	000: 576MHz
	001: 600MHz
	010: NA
	011: 480Mhz
	100: 384Mhz
	101: 320Mhz
	110: 240Mhz
	111: 192Mhz*/
	#define offset_of_clkgen2_reg_ckg_evd_ppu (56)
	#define mask_of_clkgen2_reg_ckg_evd_ppu (0x1f)
	unsigned int reg_ckg_evd_ppu:5;

	// h001c, bit: 14
	/* */
	unsigned int :11;

	// h001c
	unsigned int /* padding 16 bit */:16;

	// h001d, bit: 3
	/* CLK_VP8 clock settings
	[0] : disable clock
	[1] : invert clock
	[3:2] : Select clock source
	  00 : 288 MHz
	  01 : 240 MHz
	  10 : 216 MHz
	  11 : 320 MHz*/
	#define offset_of_clkgen2_reg_ckg_vp8 (58)
	#define mask_of_clkgen2_reg_ckg_vp8 (0xf)
	unsigned int reg_ckg_vp8:4;

	// h001d, bit: 14
	/* */
	unsigned int :12;

	// h001d
	unsigned int /* padding 16 bit */:16;

	// h001e, bit: 14
	/* */
	unsigned int :16;

	// h001e
	unsigned int /* padding 16 bit */:16;

	// h001f, bit: 14
	/* */
	unsigned int :16;

	// h001f
	unsigned int /* padding 16 bit */:16;

	// h0020, bit: 4
	/* CLK_MIU_256BUS clock setting
	[0]: disable clock
	[1]: invert clock
	[3:2]: Select clock source
	    00:   clk_miu0_256bus_highway_1x_p
	    01:   288 MHz
	    10:   MIU_128BUS_PLL_CLK_VCO_OUT
	    11:   216 MHz
	[4]: select HEMCU fast way
	     0: clock select by [3:2]
	     1: clock for HEMCU fast way*/
	#define offset_of_clkgen2_ reg_ckg_miu_256bus (64)
	#define mask_of_clkgen2_ reg_ckg_miu_256bus (0x1f)
	unsigned int  reg_ckg_miu_256bus:5;

	// h0020, bit: 14
	/* */
	unsigned int :11;

	// h0020
	unsigned int /* padding 16 bit */:16;

	// h0021, bit: 14
	/* */
	unsigned int :16;

	// h0021
	unsigned int /* padding 16 bit */:16;

	// h0022, bit: 14
	/* */
	unsigned int :16;

	// h0022
	unsigned int /* padding 16 bit */:16;

	// h0023, bit: 14
	/* */
	unsigned int :16;

	// h0023
	unsigned int /* padding 16 bit */:16;

	// h0024, bit: 14
	/* */
	unsigned int :16;

	// h0024
	unsigned int /* padding 16 bit */:16;

	// h0025, bit: 3
	/*  clock settings
	[0] : disable clock
	[1] : invert clock
	[3:2] : Select clock source
	  00 :  MHz
	  01 :  MHz
	  10 :  MHz
	  11 :  MHz*/
	#define offset_of_clkgen2_reg_ckg_hdr_lut (74)
	#define mask_of_clkgen2_reg_ckg_hdr_lut (0xf)
	unsigned int reg_ckg_hdr_lut:4;

	// h0025, bit: 7
	/*  clock settings
	[0] : disable clock
	[1] : invert clock
	[3:2] : Select clock source
	  00 :  MHz
	  01 :  MHz
	  10 :  MHz
	  11 :  MHz*/
	#define offset_of_clkgen2_reg_ckg_hdr_lut_sram (74)
	#define mask_of_clkgen2_reg_ckg_hdr_lut_sram (0xf0)
	unsigned int reg_ckg_hdr_lut_sram:4;

	// h0025, bit: 10
	/* clock setting
	000:   MHz
	001:   MHz
	010:   MHz
	100:   MHz*/
	#define offset_of_clkgen2_reg_ckg_xvycc_sram (74)
	#define mask_of_clkgen2_reg_ckg_xvycc_sram (0x700)
	unsigned int reg_ckg_xvycc_sram:3;

	// h0025, bit: 14
	/* */
	unsigned int :5;

	// h0025
	unsigned int /* padding 16 bit */:16;

	// h0026, bit: 14
	/* */
	unsigned int :16;

	// h0026
	unsigned int /* padding 16 bit */:16;

	// h0027, bit: 14
	/* */
	unsigned int :16;

	// h0027
	unsigned int /* padding 16 bit */:16;

	// h0028, bit: 14
	/* */
	unsigned int :16;

	// h0028
	unsigned int /* padding 16 bit */:16;

	// h0029, bit: 14
	/* */
	unsigned int :16;

	// h0029
	unsigned int /* padding 16 bit */:16;

	// h002a, bit: 14
	/* */
	unsigned int :16;

	// h002a
	unsigned int /* padding 16 bit */:16;

	// h002b, bit: 14
	/* */
	unsigned int :16;

	// h002b
	unsigned int /* padding 16 bit */:16;

	// h002c, bit: 14
	/* */
	unsigned int :16;

	// h002c
	unsigned int /* padding 16 bit */:16;

	// h002d, bit: 14
	/* */
	unsigned int :16;

	// h002d
	unsigned int /* padding 16 bit */:16;

	// h002e, bit: 14
	/* */
	unsigned int :16;

	// h002e
	unsigned int /* padding 16 bit */:16;

	// h002f, bit: 14
	/* */
	unsigned int :16;

	// h002f
	unsigned int /* padding 16 bit */:16;

	// h0030, bit: 4
	/* [0] : disable clock
	clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_pm (96)
	#define mask_of_clkgen2_reg_ckg_bist_pm (0x1f)
	unsigned int reg_ckg_bist_pm:5;

	// h0030, bit: 14
	/* */
	unsigned int :11;

	// h0030
	unsigned int /* padding 16 bit */:16;

	// h0031, bit: 4
	/* [0] : disable clock
	clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_codec (98)
	#define mask_of_clkgen2_reg_ckg_bist_codec (0x1f)
	unsigned int reg_ckg_bist_codec:5;

	// h0031, bit: 14
	/* */
	unsigned int :11;

	// h0031
	unsigned int /* padding 16 bit */:16;

	// h0032, bit: 4
	/* [0] : disable clock
	clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_hi_codec (100)
	#define mask_of_clkgen2_reg_ckg_bist_hi_codec (0x1f)
	unsigned int reg_ckg_bist_hi_codec:5;

	// h0032, bit: 14
	/* */
	unsigned int :11;

	// h0032
	unsigned int /* padding 16 bit */:16;

	// h0033, bit: 4
	/* [0] : disable clock
	clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_sc0 (102)
	#define mask_of_clkgen2_reg_ckg_bist_sc0 (0x1f)
	unsigned int reg_ckg_bist_sc0:5;

	// h0033, bit: 14
	/* */
	unsigned int :11;

	// h0033
	unsigned int /* padding 16 bit */:16;

	// h0034, bit: 4
	/* [0] : disable clock
	clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_sc1 (104)
	#define mask_of_clkgen2_reg_ckg_bist_sc1 (0x1f)
	unsigned int reg_ckg_bist_sc1:5;

	// h0034, bit: 14
	/* */
	unsigned int :11;

	// h0034
	unsigned int /* padding 16 bit */:16;

	// h0035, bit: 4
	/* [0] : disable clock
	clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_tsp (106)
	#define mask_of_clkgen2_reg_ckg_bist_tsp (0x1f)
	unsigned int reg_ckg_bist_tsp:5;

	// h0035, bit: 14
	/* */
	unsigned int :11;

	// h0035
	unsigned int /* padding 16 bit */:16;

	// h0036, bit: 4
	/* [0] : disable clock
	clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_venc0 (108)
	#define mask_of_clkgen2_reg_ckg_bist_venc0 (0x1f)
	unsigned int reg_ckg_bist_venc0:5;

	// h0036, bit: 14
	/* */
	unsigned int :11;

	// h0036
	unsigned int /* padding 16 bit */:16;

	// h0037, bit: 4
	/* [0] : disable clock
	clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_venc1 (110)
	#define mask_of_clkgen2_reg_ckg_bist_venc1 (0x1f)
	unsigned int reg_ckg_bist_venc1:5;

	// h0037, bit: 14
	/* */
	unsigned int :11;

	// h0037
	unsigned int /* padding 16 bit */:16;

	// h0038, bit: 4
	/* [0] : disable clock
	clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_isp_sc (112)
	#define mask_of_clkgen2_reg_ckg_bist_isp_sc (0x1f)
	unsigned int reg_ckg_bist_isp_sc:5;

	// h0038, bit: 14
	/* */
	unsigned int :11;

	// h0038
	unsigned int /* padding 16 bit */:16;

	// h0039, bit: 4
	/* [0] : disable clock
	clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_ceva (114)
	#define mask_of_clkgen2_reg_ckg_bist_ceva (0x1f)
	unsigned int reg_ckg_bist_ceva:5;

	// h0039, bit: 14
	/* */
	unsigned int :11;

	// h0039
	unsigned int /* padding 16 bit */:16;

	// h003a, bit: 3
	/* clk_miic3 clock setting
	[0]:  disable clock
	[1]:  invert clock
	[3:2]:  Select clock sources
	   00: 72Mhz
	   01:  xtali
	   10:  36Mhz
	   11:  54Mhz*/
	#define offset_of_clkgen2_reg_ckg_miic4 (116)
	#define mask_of_clkgen2_reg_ckg_miic4 (0xf)
	unsigned int reg_ckg_miic4:4;

	// h003a, bit: 7
	/* clk_miic3 clock setting
	[0]:  disable clock
	[1]:  invert clock
	[3:2]:  Select clock sources
	   00: 72Mhz
	   01:  xtali
	   10:  36Mhz
	   11:  54Mhz*/
	#define offset_of_clkgen2_reg_ckg_miic5 (116)
	#define mask_of_clkgen2_reg_ckg_miic5 (0xf0)
	unsigned int reg_ckg_miic5:4;

	// h003a, bit: 11
	/* clk_miic3 clock setting
	[0]:  disable clock
	[1]:  invert clock
	[3:2]:  Select clock sources
	   00: 72Mhz
	   01:  xtali
	   10:  36Mhz
	   11:  54Mhz*/
	#define offset_of_clkgen2_reg_ckg_miic6 (116)
	#define mask_of_clkgen2_reg_ckg_miic6 (0xf00)
	unsigned int reg_ckg_miic6:4;

	// h003a, bit: 14
	/* */
	unsigned int :4;

	// h003a
	unsigned int /* padding 16 bit */:16;

	// h003b, bit: 3
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_ts_sample (118)
	#define mask_of_clkgen2_reg_ckg_ts_sample (0xf)
	unsigned int reg_ckg_ts_sample:4;

	// h003b, bit: 14
	/* */
	unsigned int :12;

	// h003b
	unsigned int /* padding 16 bit */:16;

	// h003c, bit: 3
	/* CLK_FCIE_SYN clock setting
	[0]:  disable clock
	[1]:  invert clock
	[3:2]:  Select clock sources
	       00:  216Mhz
	       01:  432 MHz
	       10:  XTAL
	       11:  DFT clock*/
	#define offset_of_clkgen2_reg_ckg_fcie_syn (120)
	#define mask_of_clkgen2_reg_ckg_fcie_syn (0xf)
	unsigned int reg_ckg_fcie_syn:4;

	// h003c, bit: 7
	/* CLK_SD_SYN clock setting
	[0]:  disable clock
	[1]:  invert clock
	[3:2]:  Select clock sources
	       00:  216Mhz
	       01:  432 MHz
	       10:  XTAL
	       11:  DFT clock*/
	#define offset_of_clkgen2_reg_ckg_sd_syn (120)
	#define mask_of_clkgen2_reg_ckg_sd_syn (0xf0)
	unsigned int reg_ckg_sd_syn:4;

	// h003c, bit: 14
	/* */
	unsigned int :8;

	// h003c
	unsigned int /* padding 16 bit */:16;

	// h003d, bit: 4
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_gpu (122)
	#define mask_of_clkgen2_reg_ckg_gpu (0x1f)
	unsigned int reg_ckg_gpu:5;

	// h003d, bit: 14
	/* */
	unsigned int :11;

	// h003d
	unsigned int /* padding 16 bit */:16;

	// h003e, bit: 6
	/* CLK_SDIO clock setting
	[0]: disable clock
	[1]: invert clock
	[6:2]: Select clock source
	     10000:   clk_xtali_buf
	     10001:   20  MHz
	     10010:   32   MHz
	     10011:   36   MHz
	     10100:   40   MHz
	     10101:   43.2MHz
	     10110:   54   MHz
	     10111:   62   MHz
	     11000:   72   MHz
	     11001:   86   MHz
	     11010:   5.4  Mhz
	     11011:   clk_sdio_1x_p
	     11100:   clk_sdio_2x_p
	     11101:   300 KHz
	     11110:   clk_xtali_buf
	     11111:   48   MHz
	     0xxxx: select clk_xtali_buf*/
	#define offset_of_clkgen2_reg_ckg_sdio (124)
	#define mask_of_clkgen2_reg_ckg_sdio (0x7f)
	unsigned int reg_ckg_sdio:7;

	// h003e, bit: 7
	/* */
	unsigned int :1;

	// h003e, bit: 14
	/* CLK_SD30 clock setting
	[0]: disable clock
	[1]: invert clock
	[6:2]: Select clock source
	     10000:   clk_xtali_buf
	     10001:   20  MHz
	     10010:   32   MHz
	     10011:   36   MHz
	     10100:   40   MHz
	     10101:   43.2MHz
	     10110:   54   MHz
	     10111:   62   MHz
	     11000:   72   MHz
	     11001:   86   MHz
	     11010:   5.4  Mhz
	     11011:   clk_sdio_1x_p
	     11100:   clk_sdio_2x_p
	     11101:   300 KHz
	     11110:   clk_xtali_buf
	     11111:   48   MHz
	     0xxxx: select clk_xtali_buf*/
	#define offset_of_clkgen2_reg_ckg_sd30 (124)
	#define mask_of_clkgen2_reg_ckg_sd30 (0x7f00)
	unsigned int reg_ckg_sd30:7;

	// h003e, bit: 15
	/* */
	unsigned int :1;

	// h003e
	unsigned int /* padding 16 bit */:16;

	// h003f, bit: 14
	/* */
	unsigned int :16;

	// h003f
	unsigned int /* padding 16 bit */:16;

	// h0040, bit: 0
	/* Update the control words of DC2 free-running synthesizer*/
	#define offset_of_clkgen2_reg_update_dc2_freerun_cw (128)
	#define mask_of_clkgen2_reg_update_dc2_freerun_cw (0x1)
	unsigned int reg_update_dc2_freerun_cw:1;

	// h0040, bit: 1
	/* Update the control words of DC2 synthesizer that is synchronized to STC0.*/
	#define offset_of_clkgen2_reg_update_dc2_sync_cw (128)
	#define mask_of_clkgen2_reg_update_dc2_sync_cw (0x2)
	unsigned int reg_update_dc2_sync_cw:1;

	// h0040, bit: 7
	/* */
	unsigned int :6;

	// h0040, bit: 8
	/* Update the control words of DC3 free-running synthesizer*/
	#define offset_of_clkgen2_reg_update_dc3_freerun_cw (128)
	#define mask_of_clkgen2_reg_update_dc3_freerun_cw (0x100)
	unsigned int reg_update_dc3_freerun_cw:1;

	// h0040, bit: 9
	/* Update the control words of DC3 synthesizer that is synchronized to STC1.*/
	#define offset_of_clkgen2_reg_update_dc3_sync_cw (128)
	#define mask_of_clkgen2_reg_update_dc3_sync_cw (0x200)
	unsigned int reg_update_dc3_sync_cw:1;

	// h0040, bit: 14
	/* */
	unsigned int :6;

	// h0040
	unsigned int /* padding 16 bit */:16;

	// h0041, bit: 14
	/* */
	unsigned int :16;

	// h0041
	unsigned int /* padding 16 bit */:16;

	// h0042, bit: 14
	/* control word of the synthesizer of MPEG VOP0 clocks*/
	#define offset_of_clkgen2_reg_dc2_freerun_cw (132)
	#define mask_of_clkgen2_reg_dc2_freerun_cw (0xffff)
	unsigned int reg_dc2_freerun_cw:16;

	// h0042
	unsigned int /* padding 16 bit */:16;

	// h0043, bit: 14
	/* control word of the synthesizer of MPEG VOP0 clocks*/
	#define offset_of_clkgen2_reg_dc2_freerun_cw_1 (134)
	#define mask_of_clkgen2_reg_dc2_freerun_cw_1 (0xffff)
	unsigned int reg_dc2_freerun_cw_1:16;

	// h0043
	unsigned int /* padding 16 bit */:16;

	// h0044, bit: 14
	/* Numerator of the synthesizer of DC2*/
	#define offset_of_clkgen2_reg_dc2_num (136)
	#define mask_of_clkgen2_reg_dc2_num (0xffff)
	unsigned int reg_dc2_num:16;

	// h0044
	unsigned int /* padding 16 bit */:16;

	// h0045, bit: 14
	/* Denominator of the synthesizer of DC2*/
	#define offset_of_clkgen2_reg_dc2_den (138)
	#define mask_of_clkgen2_reg_dc2_den (0xffff)
	unsigned int reg_dc2_den:16;

	// h0045
	unsigned int /* padding 16 bit */:16;

	// h0046, bit: 14
	/* control word of the synthesizer of MPEG VOP0 clocks*/
	#define offset_of_clkgen2_reg_dc3_freerun_cw (140)
	#define mask_of_clkgen2_reg_dc3_freerun_cw (0xffff)
	unsigned int reg_dc3_freerun_cw:16;

	// h0046
	unsigned int /* padding 16 bit */:16;

	// h0047, bit: 14
	/* control word of the synthesizer of MPEG VOP0 clocks*/
	#define offset_of_clkgen2_reg_dc3_freerun_cw_1 (142)
	#define mask_of_clkgen2_reg_dc3_freerun_cw_1 (0xffff)
	unsigned int reg_dc3_freerun_cw_1:16;

	// h0047
	unsigned int /* padding 16 bit */:16;

	// h0048, bit: 14
	/* Numerator of the synthesizer of DC3*/
	#define offset_of_clkgen2_reg_dc3_num (144)
	#define mask_of_clkgen2_reg_dc3_num (0xffff)
	unsigned int reg_dc3_num:16;

	// h0048
	unsigned int /* padding 16 bit */:16;

	// h0049, bit: 14
	/* Denominator of the synthesizer of DC3*/
	#define offset_of_clkgen2_reg_dc3_den (146)
	#define mask_of_clkgen2_reg_dc3_den (0xffff)
	unsigned int reg_dc3_den:16;

	// h0049
	unsigned int /* padding 16 bit */:16;

	// h004a, bit: 0
	/* STC2 synthesizer software reset, active high.*/
	#define offset_of_clkgen2_reg_stc2syn_rst (148)
	#define mask_of_clkgen2_reg_stc2syn_rst (0x1)
	unsigned int reg_stc2syn_rst:1;

	// h004a, bit: 1
	/* select from which STC2 word is controlled
	0: register controlled by House Keeping MCU
	1: register controlled by TSP*/
	#define offset_of_clkgen2_reg_stc2_cw_sel (148)
	#define mask_of_clkgen2_reg_stc2_cw_sel (0x2)
	unsigned int reg_stc2_cw_sel:1;

	// h004a, bit: 2
	/* update control word of the synthesizer STC2.*/
	#define offset_of_clkgen2_reg_update_stc2_cw (148)
	#define mask_of_clkgen2_reg_update_stc2_cw (0x4)
	unsigned int reg_update_stc2_cw:1;

	// h004a, bit: 3
	/* Enable Reference DIV2 for STC2*/
	#define offset_of_clkgen2_reg_stc2_ref_div2_en (148)
	#define mask_of_clkgen2_reg_stc2_ref_div2_en (0x8)
	unsigned int reg_stc2_ref_div2_en:1;

	// h004a, bit: 7
	/* */
	unsigned int :4;

	// h004a, bit: 8
	/* STC3 synthesizer software reset, active high.*/
	#define offset_of_clkgen2_reg_stc3syn_rst (148)
	#define mask_of_clkgen2_reg_stc3syn_rst (0x100)
	unsigned int reg_stc3syn_rst:1;

	// h004a, bit: 9
	/* select from which STC3 word is controlled
	0: register controlled by House Keeping MCU
	1: register controlled by TSP*/
	#define offset_of_clkgen2_reg_stc3_cw_sel (148)
	#define mask_of_clkgen2_reg_stc3_cw_sel (0x200)
	unsigned int reg_stc3_cw_sel:1;

	// h004a, bit: 10
	/* update control word of the synthesizer STC3.*/
	#define offset_of_clkgen2_reg_update_stc3_cw (148)
	#define mask_of_clkgen2_reg_update_stc3_cw (0x400)
	unsigned int reg_update_stc3_cw:1;

	// h004a, bit: 11
	/* Enable Reference DIV2 for STC3*/
	#define offset_of_clkgen2_reg_stc3_ref_div2_en (148)
	#define mask_of_clkgen2_reg_stc3_ref_div2_en (0x800)
	unsigned int reg_stc3_ref_div2_en:1;

	// h004a, bit: 14
	/* */
	unsigned int :4;

	// h004a
	unsigned int /* padding 16 bit */:16;

	// h004b, bit: 14
	/* control word of the synthesizer of STC2 clocks*/
	#define offset_of_clkgen2_reg_stc2syn_cw (150)
	#define mask_of_clkgen2_reg_stc2syn_cw (0xffff)
	unsigned int reg_stc2syn_cw:16;

	// h004b
	unsigned int /* padding 16 bit */:16;

	// h004c, bit: 14
	/* control word of the synthesizer of STC2 clocks*/
	#define offset_of_clkgen2_reg_stc2syn_cw_1 (152)
	#define mask_of_clkgen2_reg_stc2syn_cw_1 (0xffff)
	unsigned int reg_stc2syn_cw_1:16;

	// h004c
	unsigned int /* padding 16 bit */:16;

	// h004d, bit: 14
	/* control word of the synthesizer of STC3 clocks*/
	#define offset_of_clkgen2_reg_stc3syn_cw (154)
	#define mask_of_clkgen2_reg_stc3syn_cw (0xffff)
	unsigned int reg_stc3syn_cw:16;

	// h004d
	unsigned int /* padding 16 bit */:16;

	// h004e, bit: 14
	/* control word of the synthesizer of STC3 clocks*/
	#define offset_of_clkgen2_reg_stc3syn_cw_1 (156)
	#define mask_of_clkgen2_reg_stc3syn_cw_1 (0xffff)
	unsigned int reg_stc3syn_cw_1:16;

	// h004e
	unsigned int /* padding 16 bit */:16;

	// h004f, bit: 3
	/* CLK_STC2 clock setting
	[0]: disable clock
	[1]: invert clock
	[3:2]: Clock source
	�� 00: select STC2 synthesizer output
	�� 01: 1
	�� 10: 27MHz
	�� 11: reserved*/
	#define offset_of_clkgen2_reg_ckg_stc2 (158)
	#define mask_of_clkgen2_reg_ckg_stc2 (0xf)
	unsigned int reg_ckg_stc2:4;

	// h004f, bit: 7
	/* CLK_STC3 clock setting
	[0]: disable clock
	[1]: invert clock
	[3:2]: Clock source
	�� 00: select STC3 synthesizer output
	�� 01: 1
	�� 10: 27MHz
	�� 11: reserved*/
	#define offset_of_clkgen2_reg_ckg_stc3 (158)
	#define mask_of_clkgen2_reg_ckg_stc3 (0xf0)
	unsigned int reg_ckg_stc3:4;

	// h004f, bit: 10
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_syn_stc2 (158)
	#define mask_of_clkgen2_reg_ckg_syn_stc2 (0x700)
	unsigned int reg_ckg_syn_stc2:3;

	// h004f, bit: 11
	/* */
	unsigned int :1;

	// h004f, bit: 14
	/* no use*/
	#define offset_of_clkgen2_reg_ckg_syn_stc3 (158)
	#define mask_of_clkgen2_reg_ckg_syn_stc3 (0x7000)
	unsigned int reg_ckg_syn_stc3:3;

	// h004f, bit: 15
	/* */
	unsigned int :1;

	// h004f
	unsigned int /* padding 16 bit */:16;

	// h0050, bit: 4
	/* [0] : disable clock
	clk_bist clock setting [4::2]
	000:  216 MHz
	001:  172 MHz
	010:  144 MHz
	011:  123 MHz
	100:  108 MHz
	101:    86 MHz
	110:    62 MHz
	111:  select XTAL*/
	#define offset_of_clkgen2_reg_ckg_bist_diamond (160)
	#define mask_of_clkgen2_reg_ckg_bist_diamond (0x1f)
	unsigned int reg_ckg_bist_diamond:5;

	// h0050, bit: 7
	/* */
	unsigned int :3;

	// h0050, bit: 12
	/* clk_secgmac_ahb clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	     000:  144MHz
	     001:  172MHz
	     010:  192Mhz
	     011:  216MHz
	     100:  N/A
	     101:  N/A
	     110:  N/A
	     111:  N/A*/
	#define offset_of_clkgen2_reg_ckg_secgmac_ahb (160)
	#define mask_of_clkgen2_reg_ckg_secgmac_ahb (0x1f00)
	unsigned int reg_ckg_secgmac_ahb:5;

	// h0050, bit: 14
	/* */
	unsigned int :3;

	// h0050
	unsigned int /* padding 16 bit */:16;

	// h0051, bit: 4
	/* clk_ispsc_fclk1 clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	     000:  432MHz
	     001:  384MHz
	     010:  320Mhz
	     011:  288MHz
	     100:  240MHz
	     101:  216MHz
	     110:  123MHz
	     111:  480MHz*/
	#define offset_of_clkgen2_reg_ckg_ispsc_fclk1 (162)
	#define mask_of_clkgen2_reg_ckg_ispsc_fclk1 (0x1f)
	unsigned int reg_ckg_ispsc_fclk1:5;

	// h0051, bit: 7
	/* */
	unsigned int :3;

	// h0051, bit: 12
	/* clk_ispsc_fclk2 clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	     000:  432MHz
	     001:  384MHz
	     010:  320Mhz
	     011:  288MHz
	     100:  240MHz
	     101:  216MHz
	     110:  123MHz
	     111:  480MHz*/
	#define offset_of_clkgen2_reg_ckg_ispsc_fclk2 (162)
	#define mask_of_clkgen2_reg_ckg_ispsc_fclk2 (0x1f00)
	unsigned int reg_ckg_ispsc_fclk2:5;

	// h0051, bit: 14
	/* */
	unsigned int :3;

	// h0051
	unsigned int /* padding 16 bit */:16;

	// h0052, bit: 3
	/* clk_ispsc_odclk clock setting
	[0]: disable clock
	[1]: invert clock
	[3:2]: Clock source
	�� 00: 86MHz
	�� 01: 43MHz
	�� 10: 21.5MHz
	�� 11: 172MHz*/
	#define offset_of_clkgen2_reg_ckg_ispsc_odclk (164)
	#define mask_of_clkgen2_reg_ckg_ispsc_odclk (0xf)
	unsigned int reg_ckg_ispsc_odclk:4;

	// h0052, bit: 8
	/* clk_ive clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	     000:  172MHz
	     001:  86MHz
	     010:  216Mhz
	     011:  288MHz
	     100:  320MHz
	     101:  reserved
	     110:  reserved
	     111:  reserved*/
	#define offset_of_clkgen2_reg_ckg_ive (164)
	#define mask_of_clkgen2_reg_ckg_ive (0x1f0)
	unsigned int reg_ckg_ive:5;

	// h0052, bit: 14
	/* */
	unsigned int :7;

	// h0052
	unsigned int /* padding 16 bit */:16;

	// h0053, bit: 3
	/* clk_miu_dmagen0 clock setting
	[0]: disable clock
	[1]: invert clock
	[3:2]: Clock source
	�� 00: clk_miu
	     others reserved*/
	#define offset_of_clkgen2_reg_ckg_miu_dmagen0 (166)
	#define mask_of_clkgen2_reg_ckg_miu_dmagen0 (0xf)
	unsigned int reg_ckg_miu_dmagen0:4;

	// h0053, bit: 7
	/* clk_miu_dmagen1 clock setting
	[0]: disable clock
	[1]: invert clock
	[3:2]: Clock source
	�� 00: clk_miu
	     others reserved*/
	#define offset_of_clkgen2_reg_ckg_miu_dmagen1 (166)
	#define mask_of_clkgen2_reg_ckg_miu_dmagen1 (0xf0)
	unsigned int reg_ckg_miu_dmagen1:4;

	// h0053, bit: 12
	/* clk_isp_img clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	0: clk_432_buf
	1: clk_384_buf
	2: clk_320_buf
	3: clk_288_buf
	4: clk_240_buf
	5: clk_216_buf
	6: clk_123_buf
	7: isppll_600m_in
	*/
	#define offset_of_clkgen2_reg_ckg_isp_img (166)
	#define mask_of_clkgen2_reg_ckg_isp_img (0x1f00)
	unsigned int reg_ckg_isp_img:5;

	// h0053, bit: 14
	/* */
	unsigned int :3;

	// h0053
	unsigned int /* padding 16 bit */:16;

	// h0054, bit: 4
	/* clk_ispsc_ldc_feye clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	0 => clk_345_buf (default)
	1 => clk_320_buf
	2 => clk_288_buf
	3 => clk_240_buf
	4 => clk_216_buf
	5 => clk_123_buf
	6 => clk_432_buf
	7 => clk_384_buf*/
	#define offset_of_clkgen2_reg_ckg_ispsc_ldc_feye (168)
	#define mask_of_clkgen2_reg_ckg_ispsc_ldc_feye (0x1f)
	unsigned int reg_ckg_ispsc_ldc_feye:5;

	// h0054, bit: 14
	/* */
	unsigned int :11;

	// h0054
	unsigned int /* padding 16 bit */:16;

	// h0055, bit: 4
	/* sr0_mclk clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	0:6MHz
	1:12MHz
	2:24MHz
	3:27MHz
	4:36MHz
	5:37.5MHz
	6:54MHz
	7:75MHz*/
	#define offset_of_clkgen2_reg_ckg_sr0_mclk (170)
	#define mask_of_clkgen2_reg_ckg_sr0_mclk (0x1f)
	unsigned int reg_ckg_sr0_mclk:5;

	// h0055, bit: 7
	/* */
	unsigned int :3;

	// h0055, bit: 12
	/* sr1_mclk clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	0:6MHz
	1:12MHz
	2:24MHz
	3:27MHz
	4:36MHz
	5:37.5MHz
	6:54MHz
	7:75MHz*/
	#define offset_of_clkgen2_reg_ckg_sr1_mclk (170)
	#define mask_of_clkgen2_reg_ckg_sr1_mclk (0x1f00)
	unsigned int reg_ckg_sr1_mclk:5;

	// h0055, bit: 14
	/* */
	unsigned int :3;

	// h0055
	unsigned int /* padding 16 bit */:16;

	// h0056, bit: 4
	/* sr2_mclk clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	0:6MHz
	1:12MHz
	2:24MHz
	3:27MHz
	4:36MHz
	5:37.5MHz
	6:54MHz
	7:75MHz*/
	#define offset_of_clkgen2_reg_ckg_sr2_mclk (172)
	#define mask_of_clkgen2_reg_ckg_sr2_mclk (0x1f)
	unsigned int reg_ckg_sr2_mclk:5;

	// h0056, bit: 7
	/* */
	unsigned int :3;

	// h0056, bit: 12
	/* sr3_mclk clock setting
	[0]: disable clock
	[1]: invert clock
	[4:2]: Select clock source
	0:6MHz
	1:12MHz
	2:24MHz
	3:27MHz
	4:36MHz
	5:37.5MHz
	6:54MHz
	7:75MHz*/
	#define offset_of_clkgen2_reg_ckg_sr3_mclk (172)
	#define mask_of_clkgen2_reg_ckg_sr3_mclk (0x1f00)
	unsigned int reg_ckg_sr3_mclk:5;

	// h0056, bit: 14
	/* */
	unsigned int :3;

	// h0056
	unsigned int /* padding 16 bit */:16;

	// h0057, bit: 5
	/* */
	#define offset_of_clkgen2_reg_ckg_hdgen_mux_in (174)
	#define mask_of_clkgen2_reg_ckg_hdgen_mux_in (0x3f)
	unsigned int reg_ckg_hdgen_mux_in:6;

	// h0057, bit: 7
	/* */
	unsigned int :2;

	// h0057, bit: 12
	/* [0]: disable clock
	[1]: invert clock
	[3:2]: Select clock source
	0:lpll clk
	1:lpll clk/2
	2:lpll clk/4
	3::lpll clk/8
	[4]: odclk_sel
	0:hdmi_pll clk
	1:lpll clk*/
	#define offset_of_clkgen2_reg_ckg_odclk_mipi_2p (174)
	#define mask_of_clkgen2_reg_ckg_odclk_mipi_2p (0x1f00)
	unsigned int  reg_ckg_odclk_mipi_2p:5;

	// h0057, bit: 14
	/* */
	unsigned int :3;

	// h0057
	unsigned int /* padding 16 bit */:16;

	// h0058, bit: 3
	/* lpll_osd_clk
	[0]: disable clock
	[1]: invert clock
	[3:2]: Select clock source
	0:clk_dvi
	1:clk_dc0*/
	#define offset_of_clkgen2_reg_ckg_idclk_lpll (176)
	#define mask_of_clkgen2_reg_ckg_idclk_lpll (0xf)
	unsigned int reg_ckg_idclk_lpll:4;

	// h0058, bit: 14
	/* */
	unsigned int :12;

	// h0058
	unsigned int /* padding 16 bit */:16;

	// h0059, bit: 14
	/* */
	unsigned int :16;

	// h0059
	unsigned int /* padding 16 bit */:16;

	// h005a, bit: 14
	/* */
	unsigned int :16;

	// h005a
	unsigned int /* padding 16 bit */:16;

	// h005b, bit: 14
	/* */
	unsigned int :16;

	// h005b
	unsigned int /* padding 16 bit */:16;

	// h005c, bit: 14
	/* */
	unsigned int :16;

	// h005c
	unsigned int /* padding 16 bit */:16;

	// h005d, bit: 14
	/* */
	unsigned int :16;

	// h005d
	unsigned int /* padding 16 bit */:16;

	// h005e, bit: 14
	/* */
	unsigned int :16;

	// h005e
	unsigned int /* padding 16 bit */:16;

	// h005f, bit: 14
	/* */
	unsigned int :16;

	// h005f
	unsigned int /* padding 16 bit */:16;

	// h0060, bit: 14
	/* */
	unsigned int :16;

	// h0060
	unsigned int /* padding 16 bit */:16;

	// h0061, bit: 14
	/* */
	unsigned int :16;

	// h0061
	unsigned int /* padding 16 bit */:16;

	// h0062, bit: 14
	/* */
	unsigned int :16;

	// h0062
	unsigned int /* padding 16 bit */:16;

	// h0063, bit: 14
	/* */
	unsigned int :16;

	// h0063
	unsigned int /* padding 16 bit */:16;

	// h0064, bit: 14
	/* */
	unsigned int :16;

	// h0064
	unsigned int /* padding 16 bit */:16;

	// h0065, bit: 14
	/* */
	unsigned int :16;

	// h0065
	unsigned int /* padding 16 bit */:16;

	// h0066, bit: 14
	/* */
	unsigned int :16;

	// h0066
	unsigned int /* padding 16 bit */:16;

	// h0067, bit: 14
	/* */
	unsigned int :16;

	// h0067
	unsigned int /* padding 16 bit */:16;

	// h0068, bit: 14
	/* */
	unsigned int :16;

	// h0068
	unsigned int /* padding 16 bit */:16;

	// h0069, bit: 14
	/* */
	unsigned int :16;

	// h0069
	unsigned int /* padding 16 bit */:16;

	// h006a, bit: 14
	/* */
	unsigned int :16;

	// h006a
	unsigned int /* padding 16 bit */:16;

	// h006b, bit: 14
	/* */
	unsigned int :16;

	// h006b
	unsigned int /* padding 16 bit */:16;

	// h006c, bit: 14
	/* */
	unsigned int :16;

	// h006c
	unsigned int /* padding 16 bit */:16;

	// h006d, bit: 14
	/* */
	unsigned int :16;

	// h006d
	unsigned int /* padding 16 bit */:16;

	// h006e, bit: 14
	/* */
	unsigned int :16;

	// h006e
	unsigned int /* padding 16 bit */:16;

	// h006f, bit: 14
	/* */
	unsigned int :16;

	// h006f
	unsigned int /* padding 16 bit */:16;

	// h0070, bit: 14
	/* */
	unsigned int :16;

	// h0070
	unsigned int /* padding 16 bit */:16;

	// h0071, bit: 14
	/* */
	unsigned int :16;

	// h0071
	unsigned int /* padding 16 bit */:16;

	// h0072, bit: 14
	/* */
	unsigned int :16;

	// h0072
	unsigned int /* padding 16 bit */:16;

	// h0073, bit: 14
	/* */
	unsigned int :16;

	// h0073
	unsigned int /* padding 16 bit */:16;

	// h0074, bit: 14
	/* */
	unsigned int :16;

	// h0074
	unsigned int /* padding 16 bit */:16;

	// h0075, bit: 14
	/* */
	unsigned int :16;

	// h0075
	unsigned int /* padding 16 bit */:16;

	// h0076, bit: 14
	/* */
	unsigned int :16;

	// h0076
	unsigned int /* padding 16 bit */:16;

	// h0077, bit: 14
	/* */
	unsigned int :16;

	// h0077
	unsigned int /* padding 16 bit */:16;

	// h0078, bit: 14
	/* */
	unsigned int :16;

	// h0078
	unsigned int /* padding 16 bit */:16;

	// h0079, bit: 14
	/* */
	unsigned int :16;

	// h0079
	unsigned int /* padding 16 bit */:16;

	// h007a, bit: 14
	/* */
	unsigned int :16;

	// h007a
	unsigned int /* padding 16 bit */:16;

	// h007b, bit: 14
	/* */
	unsigned int :16;

	// h007b
	unsigned int /* padding 16 bit */:16;

	// h007c, bit: 14
	/* */
	unsigned int :16;

	// h007c
	unsigned int /* padding 16 bit */:16;

	// h007d, bit: 14
	/* */
	unsigned int :16;

	// h007d
	unsigned int /* padding 16 bit */:16;

	// h007e, bit: 14
	/* */
	unsigned int :16;

	// h007e
	unsigned int /* padding 16 bit */:16;

	// h007f, bit: 14
	/* */
	unsigned int :16;

	// h007f
	unsigned int /* padding 16 bit */:16;

}  __attribute__((packed, aligned(1))) infinity2_reg_clkgen2;
#endif
