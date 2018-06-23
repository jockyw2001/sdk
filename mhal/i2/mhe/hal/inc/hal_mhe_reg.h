
#ifndef _MHE_REG_H_
#define _MHE_REG_H_

#include "mhve_ios.h"
#include "mhve_pmbr_cfg.h"


// IRQ's
#define IRQ_LESS_ROW_DONE       7
#define IRQ_NET_TRIGGER         6
#define IRQ_FS_FAIL             5
#define IRQ_TXIP_TIME_OUT       4
#define IRQ_EARLY_BSPOBUF_FULL  3
#define IRQ_IMG_BUF_FULL        2
#define IRQ_MARB_BSPOBUF_FULL   1
#define IRQ_FRAME_DONE          0

#define IRQ_MASK ((1 << IRQ_EARLY_BSPOBUF_FULL)|(1 << IRQ_FS_FAIL))

#define CHECK_IRQ_STATUS(r,irq) (((r)>>(irq))&1)


#define RIU_BANK0_SIZE 0x80
#define RIU_BANK1_SIZE 22

///////////////////////// HEV Bank0 ////////////////////////
// This map to HW excel file hev_register_definition_bank0
// RIU_BANK0_ADDR or RIU_BANK0_ADDR_CORE1

typedef struct _hev_reg_
{
    // [Video Stabilization]
    union
    {
        struct
        {
            ushort reg_w1c_hev_frame_start : 1;  //frame start (1T clk_hev) (write one clear)
            ushort reg_hev_soft_rstz : 1;        //software reset; 0/1: reset/not reset
            ushort reg_hev_ver_rid : 14;
        };
        ushort reg00;
    };
    union
    {
        struct
        {
            ushort reg01_dummy : 1;
            ushort reg_hev_enc_mode_hevc : 1;    //0/1 : other encoder/HEVC encoder
#define MHE_REG_MOD_HEVC          1
            ushort reg_hev_enc_mode : 2;         //0/1/2/3: HEVC/*/H264/JPEG
#define MHE_REG_ENC_H265          0
#define MHE_REG_ENC_H264          2
#define MHE_REG_ENC_JPEG          3
            ushort reg_hev_frame_type : 2;       //0/1/2/3: I/P/I/B
            ushort reg_hev_src_yuv_format : 2;   //Planar loader encoding format    0: 420, 1: 422
#define MHE_REG_PLNRLDR_420                   0
#define MHE_REG_PLNRLDR_422                   1
            ushort reg01_dummy1 : 1;
            ushort reg_hev_slice_id : 7;         //Slice ID
        };
        ushort reg01;
    };
    union
    {
        struct
        {
            ushort reg_hev_enc_pel_width_m1 : 14;    //encoding picture width minus1  (pels, 32byte aligned)
        };
        ushort reg02;
    };
    union
    {
        struct
        {
            ushort reg_hev_enc_pel_height_m1 : 13;   //encoding picture height minus1 (pels, 32byte aligned)
        };
        ushort reg03;
    };
    union
    {
        struct
        {
            ushort reg_hev_src_luma_pel_width : 14;  //input source picture luma width (pels, 16bytes aligned)
        };
        ushort reg04;
    };
    union
    {
        struct
        {
            ushort reg_hev_src_chroma_pel_width : 14; //input picture chroma width (pels, 16byte aligned)
            ushort reg05_dummy : 1;
            ushort reg_hev_src_chroma_swap : 1;      //source picture cb/cr swap
#define MHE_REG_PLNRLDR_UV                    0
#define MHE_REG_PLNRLDR_VU                    1
        };
        ushort reg05;
    };
    union
    {
        struct
        {
            ushort reg06_dummy : 4;
            ushort reg_hev_cur_y_adr_low : 12;
        };
        ushort reg06;
    };
    // [Frame Buffer]
    union
    {
        struct
        {
            ushort reg_hev_cur_y_adr_high : 16;      //current luma base address (byte)  16 byte aligned
        };
        ushort reg07;
    };
    union
    {
        struct
        {
            ushort reg08_dummy : 4;
            ushort reg_hev_cur_c_adr_low : 12;
        };
        ushort reg08;
    };
    union
    {
        struct
        {
            ushort reg_hev_cur_c_adr_high : 16;      //current chroma base address (byte)  16 byte aligned
        };
        ushort reg09;
    };
    union
    {
        struct
        {
            ushort reg0a_dummy : 4;
            ushort reg_hev_cur_y_adr1_low : 12;
        };
        ushort reg0a;
    };
    union
    {
        struct
        {
            ushort reg_hev_cur_y_adr1_high : 16;
        };
        ushort reg0b;
    };
    union
    {
        struct
        {
            ushort reg0c_dummy : 4;
            ushort reg_hev_cur_c_adr1_low : 12;
        };
        ushort reg0c;
    };
    union
    {
        struct
        {
            ushort reg_hev_cur_c_adr1_high : 16;
        };
        ushort reg0d;
    };
    union
    {
        struct
        {
            ushort reg_hev_enc_ctb_cnt_m1 : 16;      //encoding output CTB counts -1
        };
        ushort reg10;
    };
    union
    {
        struct
        {
            ushort reg_hev_pm_hev_d1_icg : 1;        //'b1: enabel ICG cells for power-state d1
            ushort reg11_dummy : 14;
            ushort reg_hev_pm_hev_d2_sleep : 1;      //'b1: turn off clk_hev and sleep into d2 mode, 'b0: Normal state
        };
        ushort reg11;
    };
    union
    {
        struct
        {
            ushort reg_hev_icg_me : 8;               //enable ICG cells in ME
            ushort reg_hev_icg_txip : 8;             //enable ICG cells in TXIP
        };
        ushort reg12;
    };
    union
    {
        struct
        {
            ushort reg_hev_icg_ec : 2;               //enable ICG cells in EC
        };
        ushort reg13;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_cabac_bin_cnt_low : 16; //[r]
        };
        ushort reg16;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_cabac_bin_cnt_high : 16; //[r]
        };
        ushort reg17;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_cabac_bit_cnt_low : 16; //[r]
        };
        ushort reg18;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_cabac_bit_cnt_high : 16; //[r]
        };
        ushort reg19;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_bspobuf_idx : 2;        //[r]
            ushort reg_ro_hev_ec_bspobuf_sw_idx : 2;     //[r]
            ushort reg_ro_hev_ec_bspobuf_wadr_low : 12;  //[r]
        };
        ushort reg1a;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_bspobuf_wadr_high : 16; //[r]
        };
        ushort reg1b;
    };
    // [IRQ & important IP status checkings]
    union
    {
        struct
        {
            ushort reg_hev_irq_mask : 8;                 //0/1: irq not-mask/mask
            ushort reg_hev_irq_force : 8;                //0/1: set corresponding interrupt as usual/force corresponding interrup
        };
        ushort reg1c;
    };
    union
    {
        struct
        {
            ushort reg_hev_irq_clr0 : 1;                 //0/1: not clear interrupt/clear interrupt 0 (write one clear)
            ushort reg_hev_irq_clr1 : 1;                 //0/1: not clear interrupt/clear interrupt 1 (write one clear)
            ushort reg_hev_irq_clr2 : 1;                 //0/1: not clear interrupt/clear interrupt 2 (write one clear)
            ushort reg_hev_irq_clr3 : 1;                 //0/1: not clear interrupt/clear interrupt 3 (write one clear)
            ushort reg_hev_irq_clr4 : 1;                 //0/1: not clear interrupt/clear interrupt 4 (write one clear)
            ushort reg_hev_irq_clr5 : 1;                 //0/1: not clear interrupt/clear interrupt 5 (write one clear)
            ushort reg_hev_irq_clr6 : 1;                 //0/1: not clear interrupt/clear interrupt 6 (write one clear)
            ushort reg_hev_irq_clr7 : 1;                 //0/1: not clear interrupt/clear interrupt 7 (write one clear)
            ushort reg1d_dummy : 1;
            ushort reg_hev_st_last_done_z : 1;           //[r] to indicate all miu data has been written
        };
        ushort reg1d;
    };
    union
    {
        struct
        {
            ushort reg_hev_st_irq_cpu : 8;
            ushort reg_hev_st_irq_ip : 8;
        };
        ushort reg1e;
    };
    // ME setting
    union
    {
        struct
        {
            ushort reg_hev_me_4x4_disable : 1;       //4x4_disable
            ushort reg_hev_me_8x4_disable : 1;       //8x4_disable
            ushort reg_hev_me_4x8_disable : 1;       //4x8_disable
            ushort reg_hev_me_16x8_disable : 1;      //16x8_disable
            ushort reg_hev_me_8x16_disable : 1;      //8x16_disable
            ushort reg_hev_me_8x8_disable : 1;       //8x8_disable
            ushort reg_hev_me_16x16_disable : 1;     //16x16_disable
            ushort reg_hev_mesr_adapt : 1;           //me search range auto-adaptive; 0/1: off/on
            ushort reg_hev_me_ref_en_mode : 2;       //indicating which frame(s) to be processed via ME; 2’b01/2’b10/2’b11: frame 0/ frame 1/ frame 0 & 1
        };
        ushort reg20;
    };
    // [IME PIPELINE]
    union
    {
        struct
        {
            ushort reg_hev_ime_sr16 : 1;             //search range limited to (h,v) = (+/-16, +/-16); 0/1: search range 32/16
            ushort reg_hev_ime_umv_disable : 1;      //0/1: UMV enable/disable (only disable in H263 mode)
            ushort reg_hev_ime_ime_wait_fme : 1;     //0/1: ime wait fme/fme wait ime
            ushort reg_hev_ime_boundrect_en : 1;     //ime bounding rectangle enable
        };
        ushort reg21;
    };
    union
    {
        struct
        {
            ushort reg_hev_ime_mesr_max_addr : 8;    //me search range max depth
            ushort reg_hev_ime_mesr_min_addr : 8;    //me search range min depth
        };
        ushort reg22;
    };
    union
    {
        struct
        {
            ushort reg_hev_ime_mvx_min : 8;          //me mvx min; -32 ( integer pels )
            ushort reg_hev_ime_mvx_max : 8;          //me mvx max; +31 ( integer pels )
        };
        ushort reg23;
    };
    union
    {
        struct
        {
            ushort reg_hev_ime_mvy_min : 8;          //me mvy min; -16 (integer pels )
            ushort reg_hev_ime_mvy_max : 8;          //me mvy max; +15 ( integer pels )
        };
        ushort reg24;
    };
    // [FME PIPELINE]
    union
    {
        struct
        {
            ushort reg_hev_fme_quarter_disable : 1;  //0/1: Quarter fine-tune enable/disable
            ushort reg_hev_fme_half_disable : 1;     //0/1: Half fine-tune enable/disable
            ushort reg_hev_fme_merge32_en : 1;       //0/1: disable/enable cu32 merge mode
            ushort reg_hev_fme_merge16_en : 1;       //0/1: disable/enable cu16 merge mode
            ushort reg_hev_fme_mode_no : 1;          //0: one mode.  1: two mode.
            ushort reg_hev_fme_mode0_refno : 1;      //0: one ref. for mode0   1: two ref. for mode0
            ushort reg_hev_fme_mode1_refno : 1;      //0: one ref. for mode1   1: two ref. for mode1
            ushort reg_hev_fme_mode2_refno : 1;      //0: one ref. for mode2   1: two ref. for mode2
            ushort reg_hev_fme_skip : 1;             //fme skip
            ushort reg_hev_fme_pipeline_on : 1;      //0/1: FME pipeline off/on
        };
        ushort reg25;
    };
    union
    {
        struct
        {
            ushort reg26_dummy : 2;
            ushort reg_hev_mcc_merge32_en : 1;       //0/1: disable/enable MCC cu32 merge mode
            ushort reg_hev_mcc_merge16_en : 1;       //0/1: disable/enable MCC cu16 merge mode
        };
        ushort reg26;
    };
    // [Reference address]
    union
    {
        struct
        {
            ushort reg28_dummy : 8;
            ushort reg_hev_ref_y_adr_low : 8;
        };
        ushort reg28;
    };
    union
    {
        struct
        {
            ushort reg_hev_ref_y_adr_high : 16;      //reference luma base address (*256 bytes)
        };
        ushort reg29;
    };
    union
    {
        struct
        {
            ushort reg2a_dummy : 8;
            ushort reg_hev_ref_c_adr_low : 8;
        };
        ushort reg2a;
    };
    union
    {
        struct
        {
            ushort reg_hev_ref_c_adr_high : 16;      //reference chroma base address (*256 bytes)
        };
        ushort reg2b;
    };
    union
    {
        struct
        {
            ushort reg2c_dummy : 8;
            ushort reg_hev_ref_y_adr1_low : 8;
        };
        ushort reg2c;
    };
    union
    {
        struct
        {
            ushort reg_hev_ref_y_adr1_high : 16;     //reference luma base address1 (*256 bytes)
        };
        ushort reg2d;
    };
    union
    {
        struct
        {
            ushort reg2e_dummy : 8;
            ushort reg_hev_ref_c_adr1_low : 8;
        };
        ushort reg2e;
    };
    union
    {
        struct
        {
            ushort reg_hev_ref_c_adr1_high : 16;     //reference chroma base address1 (*256 bytes)
        };
        ushort reg2f;
    };
    // [EC Setting]
    union
    {
        struct
        {
            ushort reg_w1c_hev_ec_bspobuf_set_adr : 1;               //set bsp obuf start address (write one clear)
            ushort reg_hev_ec_bspobuf_fifo_th : 3;                   //bsp obuf threshold
            ushort reg_w1c_hev_ec_bspobuf_toggle_obuf0_status : 1;   //toggle buf0 status (write one clear)
            ushort reg_w1c_hev_ec_bspobuf_toggle_obuf1_status : 1;   //toggle buf1 status (write one clear)
            ushort reg_hev_ec_bspobuf_hw_en : 1;                     //enable HW obuf automatic mechanism
        };
        ushort reg30;
    };
    union
    {
        struct
        {
            ushort reg_hev_ec_bspobuf_adr_low : 16;
        };
        ushort reg31;
    };
    union
    {
        struct
        {
            ushort reg_hev_ec_bspobuf_adr_high : 13;                 //bsp obuf address (16 byte unit)
            ushort reg32_dummy : 1;
            ushort reg_hev_ec_bspobuf_id : 2;                        //bsp obuf index
        };
        ushort reg32;
    };
    union
    {
        struct
        {
            ushort reg_hev_ec_qp_delta_enable_flag : 1;              //enable cu_qp_delta_enable_flag constant qp flag
            ushort reg_hev_ec_bspobuf_adr_rchk_sel : 2;              //read checker select
            ushort reg_hev_ec_bsp_fdc_skip : 1;                      //FDC skip enable
            ushort reg_hev_ec_bsp_fdc_offset : 7;                    //FDC offset
        };
        ushort reg33;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_ctbx : 8;                           //[r] read ec ctbx
            ushort reg_ro_hev_ec_ctby : 8;                           //[r] read ec ctby
        };
        ushort reg34;
    };
    // [CTU GN]
    union
    {
        struct
        {
            ushort reg_hev_gn_sz_ctb_m1 : 8;                         //GN memory ctb32 width, should be more than encoding ctb32 width
            ushort reg_hev_gn_sz_height : 2;                         //GN memory size height, 0/1/2/3 :  4/8/16/xx lines
        };
        ushort reg35;
    };
    union
    {
        struct
        {
            ushort reg_hev_gn_mem_simi : 1;                          //GN redirect to IMI
            ushort reg36_dummy : 3;
            ushort reg_hev_gn_mem_sadr_low : 12;
        };
        ushort reg36;
    };
    union
    {
        struct
        {
            ushort reg_hev_gn_mem_sadr_high : 16;                    //GN start address in DRAM or IMI
        };
        ushort reg37;
    };
    union
    {
        struct
        {
            ushort reg_hev_col_w_en : 1;                             //Enable write Col-located MV to DRAM
            ushort reg38_dummy : 3;
            ushort reg_hev_col_w_sadr_low : 12;
        };
        ushort reg38;
    };
    union
    {
        struct
        {
            ushort reg_hev_col_w_sadr_high : 16;                     //Col-located MV write starting address (byte)
        };
        ushort reg39;
    };
    union
    {
        struct
        {
            ushort reg_hev_col_r_en : 1;                             //Enable read Col-located MV to DRAM
            ushort reg3a_dummy : 3;
            ushort reg_hev_col_r_sadr0_low : 12;
        };
        ushort reg3a;
    };
    union
    {
        struct
        {
            ushort reg_hev_col_r_sadr0_high : 16;                    //Col-located MV of L0 Read starting address (byte)
        };
        ushort reg3b;
    };
    union
    {
        struct
        {
            ushort reg3c_dummy : 4;
            ushort reg_hev_col_r_sadr1_low : 12;
        };
        ushort reg3c;
    };
    union
    {
        struct
        {
            ushort reg_hev_col_r_sadr1_high : 16;                    //Col-located MV of L1 Read starting address (byte)
        };
        ushort reg3d;
    };
    // [MVC]
    union
    {
        struct
        {
            ushort reg_hev_cur_poc_low : 16;
        };
        ushort reg40;
    };
    union
    {
        struct
        {
            ushort reg_hev_cur_poc_high : 16;                        //Current top POC Value HM: getPOC()
        };
        ushort reg41;
    };
    union
    {
        struct
        {
            ushort reg_hev_reflst0_poc_low : 16;
        };
        ushort reg42;
    };
    union
    {
        struct
        {
            ushort reg_hev_reflst0_poc_high : 16;                    //L0 reference frame POC of list 0
        };
        ushort reg43;
    };
    union
    {
        struct
        {
            ushort reg_hev_reflst1_poc_low : 16;
        };
        ushort reg44;
    };
    union
    {
        struct
        {
            ushort reg_hev_reflst1_poc_high : 16;                    //L0 reference frame POC of list 1 (reserve this register if only support 1 reference frame)
        };
        ushort reg45;
    };
    union
    {
        struct
        {
            ushort reg_hev_col_l0_flag : 1;                          //Col-located from L0 flag HM: getColDir()
            ushort reg_hev_temp_mvp_flag : 1;                        //Temproal MVP enable flag HM: getEnableTMVPFlag()
            ushort reg_hev_max_merge_cand_m1 : 3;                    //Maximum merge candicate list number minus 1
            ushort reg_hev_parallel_merge_level : 3;                 //Parallel merge level = log2_parallel_merge_level_minus + 2
            ushort reg_hev_ref0_act_num_m1 : 5;                      //num_ref_idx_l0_active_minus1, PPS layer define its default number but it possible be overwritten in slice layer HM: getNumRefIdxL0DefaultActive()
        };
        ushort reg46;
    };
    union
    {
        struct
        {
            ushort reg_hev_reflst0_lt_fg : 1;                        //L0 reference frame long-term flag of list 0
            ushort reg_hev_reflst0_st_fg : 1;                        //L0 reference frame short-term flag of list 0
            ushort reg_hev_reflst0_fbidx : 5;                        //L0 reference frame frame buffer index of list 0
            ushort reg47_dummy : 1;
            ushort reg_hev_reflst1_lt_fg : 1;                        //L0 reference frame long-term flag of list 1 (reserve this register if only support 1 reference frame)
            ushort reg_hev_reflst1_st_fg : 1;                        //L0 reference frame short-term flag of list 1 (reserve this register if only support 1 reference frame)
            ushort reg_hev_reflst1_fbidx : 5;                        //L0 reference frame frame buffer index of list 1 (reserve this register if only support 1 reference frame)
        };
        ushort reg47;
    };
    // [MBR]
    union
    {
        struct
        {
            ushort reg_hev_slice_qp : 6;                             //slice QP
            ushort reg48_dummy : 2;
            ushort reg_hev_txip_scaling_en : 1;                      //0 : Sclaing list off, 1 : Scaling list on (HEVC default)
        };
        ushort reg48;
    };
    // [FDC]
    union
    {
        struct
        {
            ushort reg_hev_ec_fdc_bs : 16;                           //CPU to FDC bitstream data
        };
        ushort reg49;
    };
    union
    {
        struct
        {
            ushort reg_hev_ec_fdc_bs_len : 5;                        //CPU to FDC bitstream len
            ushort reg_hev_ec_fdc_bs_count : 10;                     //CPU to FDC round count - 1
        };
        ushort reg4a;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_fdc_ack : 1;                        //[r] FDC to CPU ack 0/1: Frame data pool not empty/frame data pool empty; 48x64 bits of space
            ushort reg_w1c_hev_ec_fdc_done_clr : 1;                  //FDC done clear (write one clear)
            ushort reg_ro_hev_ec_fdc_done : 1;                       //[r] FDC done; indicate to CPU that data has been written to internal buffer
            ushort reg_w1c_hev_ec_fdc_bs_vld : 1;                    //set for bitstream write out (write one clear)
            ushort reg_hev_ec_mdc_is_idr_picture : 1;                //Represent current NAL unit is IDR
            ushort reg_hev_ec_mdc_bits_ctb_num_m1 : 4;               //(Bit number -1) bit-width of CTB count
            ushort reg_hev_ec_mdc_bits_poc_m1 : 4;                   //(Bit number -1) bit-width of POC LSB
            ushort reg_hev_ec_mdc_nuh_id_p1 : 3;                     //Represent nuh_temporal_id_plus1
        };
        ushort reg4b;
    };
    // [GDR] (Multi-Slice)
    union
    {
        struct
        {
            ushort reg_hev_ec_multislice_en : 1;                     //GDR enable
            ushort reg_hev_ec_multislice_1st_ctby : 8;               //First slice CTB height
        };
        ushort reg4c;
    };
    union
    {
        struct
        {
            ushort reg_hev_ec_multislice_ctby : 8;                   //Other slices (except first) CTB height
            ushort reg_hev_ec_mdc_nal_unit_type : 6;                 //Represent nal_unit_type
            ushort reg_hev_ec_mdc_dbf_override_flag : 1;             //Represent deblocking_filter_override_flag
            ushort reg_hev_ec_mdc_chroma_qp_flag : 1;                //Represent pps_slice_chroma_qp_offsets_present_flag
        };
        ushort reg4d;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu4_intra_c1 : 7;                     //RDO c1 for intra TU 4x4
            ushort reg50_dummy : 1;
            ushort reg_hev_rdo_tu4_inter_c1 : 7;                     //RDO c1 for inter TU 4x4
        };
        ushort reg50;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu8_intra_c1 : 7;                     //RDO c1 for intra TU 8x8
            ushort reg51_dummy : 1;
            ushort reg_hev_rdo_tu8_inter_c1 : 7;                     //RDO c1 for inter TU 8x8
        };
        ushort reg51;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu16_intra_c1 : 7;                    //RDO c1 for intra TU 16x16
            ushort reg52_dummy : 1;
            ushort reg_hev_rdo_tu16_inter_c1 : 7;                    //RDO c1 for inter TU 16x16
        };
        ushort reg52;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu32_intra_c1 : 7;                    //RDO c1 for intra TU 32x32
            ushort reg53_dummy : 1;
            ushort reg_hev_rdo_tu32_inter_c1 : 7;                    //RDO c1 for inter TU 32x32
        };
        ushort reg53;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu4_intra_c0 : 10;                    //RDO c0 for intra TU 4x4
            ushort reg_hev_rdo_tu4_intra_beta : 5;                   //RDO beta for intra TU 4x4
        };
        ushort reg54;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu4_inter_c0 : 10;                    //RDO c0 for inter TU 4x4
            ushort reg_hev_rdo_tu4_inter_beta : 5;                   //RDO beta for inter TU 4x4
        };
        ushort reg55;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu8_intra_c0 : 10;                    //RDO c0 for intra TU 8x8
            ushort reg_hev_rdo_tu8_intra_beta : 5;                   //RDO beta for intra TU 8x8
        };
        ushort reg56;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu8_inter_c0 : 10;                    //RDO c0 for inter TU 8x8
            ushort reg_hev_rdo_tu8_inter_beta : 5;                   //RDO beta for inter TU 8x8
        };
        ushort reg57;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu16_intra_c0 : 10;                   //RDO c0 for intra TU 16x16
            ushort reg_hev_rdo_tu16_intra_beta : 5;                  //RDO beta for intra TU 16x16
        };
        ushort reg58;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu16_inter_c0 : 10;                   //RDO c0 for inter TU 16x16
            ushort reg_hev_rdo_tu16_inter_beta : 5;                  //RDO beta for inter TU 16x16
        };
        ushort reg59;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu32_intra_c0 : 10;                   //RDO c0 for intra TU 32x32
            ushort reg_hev_rdo_tu32_intra_beta : 5;                  //RDO beta for intra TU 32x32
        };
        ushort reg5a;
    };
    union
    {
        struct
        {
            ushort reg_hev_rdo_tu32_inter_c0 : 10;                   //RDO c0 for inter TU 32x32
            ushort reg_hev_rdo_tu32_inter_beta : 5;                  //RDO beta for inter TU 32x32
        };
        ushort reg5b;
    };
    // [TXIP]
    union
    {
        struct
        {
            ushort reg_hev_txip_cu8_intra_lose : 1;                  //0: normal mode, 1: intra cu8 off
            ushort reg_hev_txip_cu8_inter_lose : 1;                  //0: normal mode, 1: inter cu8 off
            ushort reg_hev_txip_cu16_intra_lose : 1;                 //0: normal mode, 1: intra cu16 off
            ushort reg_hev_txip_cu16_inter_mvp_lose : 1;             //0: normal mode, 1: inter mvp mode cu16 off
            ushort reg_hev_txip_cu16_inter_merge_lose : 1;           //0: normal mode, 1: inter merge mode cu16 off
            ushort reg_hev_txip_cu16_inter_mvp_nores_lose : 1;       //0: normal mode, 1: inter mvp_nores mode cu16 off
            ushort reg_hev_txip_cu16_inter_merge_nores_lose : 1;     //0: normal mode, 1: inter merge_nores mode cu16 off
            ushort reg_hev_txip_cu32_intra_lose : 1;                 //0: normal mode, 1: intra cu32 off
            ushort reg_hev_txip_cu32_inter_merge_lose : 1;           //0: normal mode, 1: inter merge mode cu32 off
            ushort reg_hev_txip_cu32_inter_merge_nores_lose : 1;     //0: normal mode, 1: inter merge_nores mode cu32 off
            ushort reg_hev_txip_idct4_intrpt : 1;                    //[r] 0: idct value in the range; 1: idct valur out of range
            ushort reg_hev_txip_idct8_intrpt : 1;                    //[r] 0: idct value in the range; 1: idct valur out of range
            ushort reg_hev_txip_idct16_intrpt : 1;                   //[r] 0: idct value in the range; 1: idct valur out of range
            ushort reg_hev_txip_idct32_intrpt : 1;                   //[r] 0: idct value in the range; 1: idct valur out of range
        };
        ushort reg5c;
    };
    union
    {
        struct
        {
            ushort reg_hev_txip_cu8_intra_penalty : 16;              //penalty of rdcost for cu8 intra
        };
        ushort reg5d;
    };
    union
    {
        struct
        {
            ushort reg_hev_txip_cu8_inter_penalty : 16;              //penalty of rdcost for cu8 inter
        };
        ushort reg5e;
    };
    union
    {
        struct
        {
            ushort reg_hev_txip_cu16_intra_penalty : 16;             //penalty of rdcost for c16 intra
        };
        ushort reg5f;
    };
    union
    {
        struct
        {
            ushort reg_hev_txip_cu16_inter_mvp_penalty : 16;         //penalty of rdcost for cu16 inter mvp
        };
        ushort reg60;
    };
    union
    {
        struct
        {
            ushort reg_hev_txip_cu16_inter_merge_penalty : 16;       //penalty of rdcost for cu16 inter merge
        };
        ushort reg61;
    };
    union
    {
        struct
        {
            ushort reg_hev_txip_cu16_inter_mvp_nores_penalty : 16;   //penalty of rdcost for cu16 inter mvp nores
        };
        ushort reg62;
    };
    union
    {
        struct
        {
            ushort reg_hev_txip_cu16_inter_merge_nores_penalty : 16; //penalty of rdcost for cu16 inter merge nores
        };
        ushort reg63;
    };
    union
    {
        struct
        {
            ushort reg_hev_txip_cu32_intra_penalty : 16;             //penalty of rdcost for c32 intra
        };
        ushort reg64;
    };
    union
    {
        struct
        {
            ushort reg_hev_txip_cu32_inter_merge_penalty : 16;       //penalty of rdcost for cu32 inter merge
        };
        ushort reg65;
    };
    union
    {
        struct
        {
            ushort reg_hev_txip_cu32_inter_merge_nores_penalty : 16; //penalty of rdcost for cu32 inter merge nores
        };
        ushort reg66;
    };
    union
    {
        struct
        {
            ushort reg_hev_txip_intra4y_mode_cnt : 4;                //Number of 4x4 intra mode candidates encoder tried. Valid value : 4, 5, 6, 7
        };
        ushort reg67;
    };
    // [PPU]
    union
    {
        struct
        {
            ushort reg68_dummy : 9;
            ushort reg_hev_ppu_fb_b_y_base_low : 7;
        };
        ushort reg68;
    };
    union
    {
        struct
        {
            ushort reg_hev_ppu_fb_b_y_base_high : 16;                //PPU intermedia B data address (*512 bytes) size = (width_in 32pixels)*16
        };
        ushort reg69;
    };
    union
    {
        struct
        {
            ushort reg6a_dummy : 9;
            ushort reg_hev_ppu_fb_a_y_base_low : 7;
        };
        ushort reg6a;
    };
    union
    {
        struct
        {
            ushort reg_hev_ppu_fb_a_y_base_high : 16;                //PPU intermedia A data address (*512 bytes) size = (height_in_32pixels)*16
        };
        ushort reg6b;
    };
    union
    {
        struct
        {
            ushort reg_hev_beta_offset : 5;
            ushort reg_hev_tc_offset : 5;
            ushort reg_hev_sao_luma_flag : 1;
            ushort reg_hev_sao_chroma_flag : 1;
            ushort reg_hev_ppu_debug_sel : 3;
        };
        ushort reg6c;
    };
    union
    {
        struct
        {
            ushort reg_hev_dissao_idc : 7;
            ushort reg6d_dummy : 1;
            ushort reg_hev_disilf_idc : 7;
        };
        ushort reg6d;
    };
    union
    {
        struct
        {
            ushort reg_hev_cb_qp_offset : 5;
            ushort reg_hev_cr_qp_offset : 5;
        };
        ushort reg6e;
    };
    union
    {
        struct
        {
            ushort reg_hev_ppu_fb_pitch : 8;                 //ppu_frame buffer pitch value = width_in_pixel / 32
            ushort reg_hev_ppu_fb_pitch_lsb : 6;             //ppu_frame buffer pitch (LSB) value = ((width_in_32pixels % 4) == 0) ? width_in_32pixels*32/128 : (width_in_32pixels+3)*32/128
        };
        ushort reg6f;
    };
    union
    {
        struct
        {
            ushort reg70_dummy : 9;
            ushort reg_hev_ppu_fblut_luma_base_low : 7;
        };
        ushort reg70;
    };
    union
    {
        struct
        {
            ushort reg_hev_ppu_fblut_luma_base_high : 16;        //PPU frame buffer luma data address (*512 bytes) size = (width_in 32pixels*32*height_in_32pixels*32)/512
        };
        ushort reg71;
    };
    union
    {
        struct
        {
            ushort reg72_dummy : 9;
            ushort reg_hev_ppu_fblut_chroma_base_low : 7;
        };
        ushort reg72;
    };
    union
    {
        struct
        {
            ushort reg_hev_ppu_fblut_chroma_base_high : 16;      //PPU frame buffer chroma data address (*512 bytes) size = ((width_in 32pixels*32*height_in_32pixels*32)/512)/2
        };
        ushort reg73;
    };
    union
    {
        struct
        {
            ushort reg74_dummy : 9;
            ushort reg_hev_ppu_fblut_luma_base_lsb_low : 7;
        };
        ushort reg74;
    };
    union
    {
        struct
        {
            ushort reg_hev_ppu_fblut_luma_base_lsb_high : 16;    //PPU frame buffer luma data address (LSB) (*512 bytes) size = (reg_hev_ppu_fb_pitch_lsb*32*height_in_32pixels*32)/512
        };
        ushort reg75;
    };
    union
    {
        struct
        {
            ushort reg76_dummy : 9;
            ushort reg_hev_ppu_fblut_chroma_base_lsb_low : 7;
        };
        ushort reg76;
    };
    union
    {
        struct
        {
            ushort reg_hev_ppu_fblut_chroma_base_lsb_high : 16;      //PPU frame buffer chroma data address (LSB) (*512 bytes) size = (reg_hev_ppu_fblut_luma_base_lsb)/2
        };
        ushort reg77;
    };
    // [Reserved]
    union
    {
        struct
        {
            ushort reg_hev_dbg_sel : 8;                              //dbg_hev_top bus sel
            ushort reg_hev_txip_dbg_sel : 7;                         //TXIP dbg_hev_top bus sel
        };
        ushort reg79;
    };
    union
    {
        struct
        {
            ushort reg_hev_dbg_bus_out_low : 16;
        };
        ushort reg7a;
    };
    union
    {
        struct
        {
            ushort reg_hev_dbg_bus_out_high : 16;                    //[r]
        };
        ushort reg7b;
    };
    union
    {
        struct
        {
            ushort reg_hev_rsv7c : 16;                               //reserved registers
        };
        ushort reg7c;
    };
    union
    {
        struct
        {
            ushort reg_hev_rsv7d : 16;                               //reserved registers
        };
        ushort reg7d;
    };
    union
    {
        struct
        {
            ushort reg_hev_rsv7e : 16;                               //reserved registers
        };
        ushort reg7e;
    };
    union
    {
        struct
        {
            ushort reg_hev_rsv7f : 16;                               //reserved registers
        };
        ushort reg7f;
    };
} HEVRegTable;

///////////////////////// MHE Bank0 ////////////////////////
// This map to HW excel file mhe_register_defintion_bank0
//RIU_MARB_ADDR or RIU_MARB_ADDR_CORE1
typedef struct _mhe_reg_
{
    union
    {
        struct
        {
            ushort reg_mhe_marb_rp_imi_en : 16;      //'[15:8]: reserved, [7:0]: rp7-rp0 imi enable
        };
        ushort reg00;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_wp_imi_en : 16;      //'[15:8]: reserved, [7:0]: wp7-wp0 imi enable
        };
        ushort reg01;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_mode_1 : 8;           //[4]: crc enable, [3:0]: crc in select d0~d7(rp0~rp7), d8~d15(wp0~wp7)
            ushort reg_mhe_hist_burst_msb_sel : 3;   //Histogram msb select, 'h0: burst_1, 'h1: burst_2, 'h2: burst_4, 'h3: burst_8, 'h4: burst_16, 'h5: burst_32, other: reserved
            ushort reg02_dummy : 3;
            ushort reg_mhe_crc_result_1_sel : 2;     //'h0: [63:0]  'h1: [127:64]  'h2: [191:128]  'h3: [255:192]
        };
        ushort reg02;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_1_rd0 : 16;
        };
        ushort reg03;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_1_rd1 : 16;
        };
        ushort reg04;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_1_rd2 : 16;
        };
        ushort reg05;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_1_rd3 : 16;
        };
        ushort reg06;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_1_rd20 : 16;
        };
        ushort reg07;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_1_rd21 : 16;
        };
        ushort reg08;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_1_rd22 : 16;
        };
        ushort reg09;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_1_rd23 : 16;
        };
        ushort reg0a;
    };
    union
    {
        struct
        {
            ushort reg_mhe_hist_burst_1 : 16;        //[r] Histogram of burst 1
        };
        ushort reg0b;
    };
    union
    {
        struct
        {
            ushort reg_mhe_hist_burst_2 : 16;        //[r] Histogram of burst 2
        };
        ushort reg0c;
    };
    union
    {
        struct
        {
            ushort reg_mhe_hist_burst_4 : 16;        //[r] Histogram of burst 4
        };
        ushort reg0d;
    };
    union
    {
        struct
        {
            ushort reg_mhe_hist_burst_8 : 16;        //[r] Histogram of burst 8
        };
        ushort reg0e;
    };
    union
    {
        struct
        {
            ushort reg_mhe_hist_burst_16 : 16;       //[r] Histogram of burst 16
        };
        ushort reg0f;
    };
    union
    {
        struct
        {
            ushort reg_mhe_hist_burst_32 : 8;        //[r] Histogram of burst 32
            ushort reg_mhe_hist_burst_msb : 6;       //[r] Histogram msb
            ushort reg_mhe_hist_sel : 2;             //Histogram select, 'h0: emi read, 'h1: emi write, 'h2: imi read, 'h3: imi write
        };
        ushort reg10;
    };
    union
    {
        struct
        {
            ushort reg_mhe_pm_miu_d1_icg : 1;        //'b1: enabel ICG cells for power-state d1
            ushort reg11_dummy : 14;
            ushort reg_mhe_pm_miu_d2_sleep : 1;      //'b1:  turn off clk_miu and sleep into d2 mode, 'b0: Normal state
        };
        ushort reg11;
    };
    // [Security]
    union
    {
        struct
        {
            ushort reg_mhe_bspobuf_adr_low : 16;
        };
        ushort reg14;
    };
    union
    {
        struct
        {
            ushort reg_mhe_bspobuf_adr_high : 13;    //bsp obuf address(8 byte unit)
            ushort reg15_dummy : 1;
            ushort reg_mhe_bspobuf_id : 2;           //bsp obuf index(2 bits)
        };
        ushort reg15;
    };
    union
    {
        struct
        {
            ushort reg_mhe_bspobuf_write_id_adr : 1; //write to this address to enable witring of bspobuf address
            ushort reg_mhe_miu_sel : 1;              //select miu bank; 0: bank0, 1:bank1
            ushort reg16_dummy : 5;
            ushort reg_mhe_ns : 9;                   //[r] Trustzone from RIU bridge
        };
        ushort reg16;
    };
    // One way bit
    union
    {
        struct
        {
            ushort reg_mhe_secure_obufadr : 1;       //Enable security control (Obuf adr)
            ushort reg_mhe_secure_miu_sel : 1;       //Enable security control (MIU sel)
            ushort reg_mhe_tlb : 1;                  //MIU TLB remap enable
        };
        ushort reg17;
    };
    // Security Bank Select
    union
    {
        struct
        {
            ushort reg_mhe_secure_bank_sel : 1;      //select security bank; 0: bank0, 1:bank1
        };
        ushort reg18;
    };
    // [IRQ & important IP status checkings]
    union
    {
        struct
        {
            ushort reg_mhe_irq_mask : 8;             //0/1: irq not-mask/mask
            ushort reg_mhe_irq_force : 8;            //0/1: set corresponding interrupt as usual/force corresponding interrupt
        };
        ushort reg1c;
    };
    union
    {
        struct
        {
            ushort reg_mhe_irq_clr0 : 1;             //0/1: not clear interrupt/clear interrupt 0 (write one clear)
            ushort reg_mhe_irq_clr1 : 1;             //0/1: not clear interrupt/clear interrupt 1 (write one clear)
            ushort reg_mhe_irq_clr2 : 1;             //0/1: not clear interrupt/clear interrupt 2 (write one clear)
            ushort reg_mhe_irq_clr3 : 1;             //0/1: not clear interrupt/clear interrupt 3 (write one clear)
            ushort reg_mhe_irq_clr4 : 1;             //0/1: not clear interrupt/clear interrupt 4 (write one clear)
            ushort reg_mhe_irq_clr5 : 1;             //0/1: not clear interrupt/clear interrupt 5 (write one clear)
            ushort reg_mhe_irq_clr6 : 1;             //0/1: not clear interrupt/clear interrupt 6 (write one clear)
            ushort reg_mhe_irq_clr7 : 1;             //0/1: not clear interrupt/clear interrupt 7 (write one clear)
            ushort reg_mhe_swrst_safe : 1;           //[r] to indicate there're no miu activities that need to pay attention to
            ushort reg_mhe_last_done_z : 1;          //[r] to indicate all miu data has been written
        };
        ushort reg1d;
    };
    union
    {
        struct
        {
            ushort reg_mhe_irq_cpu : 8; //[r] status of interrupt on CPU side ({less_row_down_irq, irq_bit6, fs_fail_irq, irq_bit4, early_bspobuf_full_irq/buf1_full, img_buf_full_irq, marb_bspobuf_full/buf0_full, frame_done_irq})
            //[6] (reg_mhe_row_down_irq_en)? viu2mhe_row_done_mhe: net_trigger;
            //[4] (reg_mhe_vs_irq_en)? vs: txip_time_out_irq;
            //[3] SW mode: early obuf full; HW mode: buf1 full
            //[1] SW mode: buf full; HW mode: buf0 full
            ushort reg_mhe_irq_ip : 8; //[r] status of interrupt on IP side ({less_row_down_irq, irq_bit6, fs_fail_irq, irq_bit4, early_bspobuf_full_irq/buf1_full, img_buf_full_irq, marb_bspobuf_full/buf0_full, frame_done_irq})
            //[6] (reg_mhe_row_down_irq_en)? viu2mhe_row_done_mhe: net_trigger;
            //[4] (reg_mhe_vs_irq_en)? vs: txip_time_out_irq;
            //[3] SW mode: early obuf full; HW mode: buf1 full
            //[1]  SW mode: buf full; HW mode: buf0 full
        };
        ushort reg1e;
    };
    // [Stall & Stall Finish]
    union
    {
        struct
        {
            ushort reg_mhe_marb_stall_finish : 1;    //[r] miu stall finish
            ushort reg_mhe_marb_stall : 1;           //[r] miu stall
        };
        ushort reg1f;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_4_low : 16;
        };
        ushort reg20;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_4_high : 12;  //miu write protection, miu upper bound 4
        };
        ushort reg21;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_4_low : 16;
        };
        ushort reg22;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_4_high : 12;  //miu write protection, miu lower bound 4
            ushort reg23_dummy : 1;
            ushort reg_mhe_marb_miu_bound_en_4 : 1;  //miu write protection, miu bound enable for write port 4
        };
        ushort reg23;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_5_low : 16;
        };
        ushort reg24;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_5_high : 12;  //miu write protection, miu upper bound 5
        };
        ushort reg25;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_5_low : 16;
        };
        ushort reg26;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_5_high : 12;  //miu write protection, miu lower bound 5
            ushort reg27_dummy : 1;
            ushort reg_mhe_marb_miu_bound_en_5 : 1;  //miu write protection, miu bound enable for write port 5
        };
        ushort reg27;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_6_low : 16;
        };
        ushort reg28;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_6_high : 12;  //miu write protection, miu upper bound 6
        };
        ushort reg29;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_6_low : 16;
        };
        ushort reg2a;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_6_high : 12;  //miu write protection, miu lower bound 6
            ushort reg2b_dummy : 1;
            ushort reg_mhe_marb_miu_bound_en_6 : 1;  //miu write protection, miu bound enable for write port 6
        };
        ushort reg2b;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_7_low : 16;
        };
        ushort reg2c;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_7_high : 12;  //miu write protection, miu upper bound 7
        };
        ushort reg2d;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_7_low : 16;
        };
        ushort reg2e;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_7_high : 12;  //miu write protection, miu lower bound 7
            ushort reg2f_dummy : 1;
            ushort reg_mhe_marb_miu_bound_en_7 : 1;  //miu write protection, miu bound enable for write port 7
        };
        ushort reg2f;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_rp0_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_rp1_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_rp2_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_rp3_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
        };
        ushort reg30;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_rp0_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_rp1_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_rp2_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_rp3_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
        };
        ushort reg31;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_wp0_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_wp1_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_wp2_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_wp3_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
        };
        ushort reg32;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_wp0_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_wp1_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_wp2_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_wp3_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
        };
        ushort reg33;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_rp4_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_rp5_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_rp6_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_rp7_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
        };
        ushort reg34;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_rp4_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_rp5_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_rp6_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_rp7_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
        };
        ushort reg35;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_wp4_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_wp5_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_wp6_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
            ushort reg_mhe_marb_wp7_occupy : 4;      //occupy count; 0: disable (default client when no other clients), 1: 16 times MIU request, 2: 32 times MIU request
        };
        ushort reg36;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_wp4_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_wp5_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_wp6_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
            ushort reg_mhe_marb_wp7_promote : 4;     //wait cycle counts after MIU request; 0: disable, 1: 4 times MIU request, 2: 8 times MIU request
        };
        ushort reg37;
    };
    // [Write Performance]
    union
    {
        struct
        {
            ushort reg_mhe_whist_en : 1;             //Write histogram enable
            ushort reg_mhe_whist_count_range : 1;    //Write histogram count range or total
            ushort reg_mhe_whist_mode : 2;           //Write histogram mode, 0: Request count, 1: Reserved, 2: Latch count, 3: Latch count without first data
            ushort reg_mhe_whist_burst_mode : 1;     //Write histogram burst count mode, 0: Count request, 1: Count burst
            ushort reg_mhe_whist_upper_bound_en : 1; //Write histogram upper bound enable
            ushort reg_mhe_whist_lower_bound_en : 1; //Write histogram lower bound enable
        };
        ushort reg40;
    };
    union
    {
        struct
        {
            ushort reg_mhe_whist_upper_bound : 16;   //Write histogram upper bound
        };
        ushort reg41;
    };
    union
    {
        struct
        {
            ushort reg_mhe_whist_lower_bound : 16;   //Write histogram lower bound
        };
        ushort reg42;
    };
    union
    {
        struct
        {
            ushort reg_mhe_whist_count_bubble : 16;  //[r] Write histogram bubble count
        };
        ushort reg43;
    };
    union
    {
        struct
        {
            ushort reg_mhe_whist_count_burst : 16;   //[r] Write histogram burst count
        };
        ushort reg44;
    };
    // [Read Performance]
    union
    {
        struct
        {
            ushort reg_mhe_rhist_en : 1;             //Read histogram enable
            ushort reg_mhe_rhist_count_range : 1;    //Read histogram count range or total
            ushort reg_mhe_rhist_mode : 2;           //Read histogram mode, 0: Request count, 1: Reserved, 2: Latch count, 3: Latch count without first data
            ushort reg_mhe_rhist_burst_mode : 1;     //Read histogram burst count mode, 0: Count request, 1: Count burst
            ushort reg_mhe_rhist_upper_bound_en : 1; //Read histogram upper bound enable
            ushort reg_mhe_rhist_lower_bound_en : 1; //Read histogram lower bound enable
        };
        ushort reg45;
    };
    union
    {
        struct
        {
            ushort reg_mhe_rhist_upper_bound : 16;   //Read histogram upper bound
        };
        ushort reg46;
    };
    union
    {
        struct
        {
            ushort reg_mhe_rhist_lower_bound : 16;   //Read histogram lower bound
        };
        ushort reg47;
    };
    union
    {
        struct
        {
            ushort reg_mhe_rhist_count_bubble : 16;  //[r] Read histogram bubble count
        };
        ushort reg48;
    };
    union
    {
        struct
        {
            ushort reg_mhe_rhist_count_burst : 16;   //[r] Read histogram burst count
        };
        ushort reg49;
    };
    union
    {
        struct
        {
            ushort reg_mhe_imi_whist_count_bubble : 16;  //[r] IMI write histogram bubble count
        };
        ushort reg4a;
    };
    union
    {
        struct
        {
            ushort reg_mhe_imi_whist_count_burst : 16;   //[r] IMI write histogram burst count
        };
        ushort reg4b;
    };
    union
    {
        struct
        {
            ushort reg_mhe_imi_rhist_count_bubble : 16;  //[r] IMI read histogram bubble count
        };
        ushort reg4c;
    };
    union
    {
        struct
        {
            ushort reg_mhe_imi_rhist_count_burst : 16;   //[r] IMI read histogram burst count
        };
        ushort reg4d;
    };
    // [SRAM ]
    union
    {
        struct
        {
            ushort reg_mhe_sram_sd_en_low : 16;
        };
        ushort reg4e;
    };
    union
    {
        struct
        {
            ushort reg_mhe_sram_sd_en_high : 14;     //sram sd enable
        };
        ushort reg4f;
    };
    // [Miu Arbiter]
    union
    {
        struct
        {
            ushort reg_mhe_marb_fdone_chk_en : 8;    //marb frame done chk bit enable
        };
        ushort reg51;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_mr_burst_thd : 5;    //miu read burst bunch up threshold
            ushort reg_mhe_marb_mr_timeout : 3;      //miu read burst timeout
            ushort reg_mhe_marb_mw_burst_thd : 5;    //miu write burst bunch up threshold
            ushort reg_mhe_marb_mw_timeout : 3;      //miu write burst timeout
        };
        ushort reg54;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_mrpriority_thd : 4;  //hardware mhe2mi_rpriority threshold
            ushort reg_mhe_marb_mwpriority_thd : 4;  //hardware mhe2mi_wpriority threshold
        };
        ushort reg55;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_mrpriority_sw : 2;   //mhe2mi_rpriority software programmable
            ushort reg_mhe_marb_mr_timeout_ref : 1;  //miu read burst timeout count start point
            ushort reg_mhe_marb_mr_nwait_mw : 1;     //miu read not wait mi2mhe_wrdy
            ushort reg_mhe_marb_mwpriority_sw : 2;   //mhe2mi_wpriority software programmable
            ushort reg_mhe_marb_mw_timeout_ref : 1;  //miu write burst timeout count start point
            ushort reg_mhe_marb_mw_nwait_mr : 1;     //miu read not wait mi2mhe_wrdy
            ushort reg_mhe_marb_mr_pending : 4;      //max. pending read requests to miu
            ushort reg_mhe_marb_32b_ad_nswap : 1;    //32bits miu address not swap. only for 32bits mode
            ushort reg_mhe_marb_miu_wmode : 1;       //0/1: original miu protocol/new miu protocol(wd_en)
            ushort reg_mhe_marb_rp_ordering : 1;     //'b0: normal, 'b1: ordering rport according to MB number and MB Stages
        };
        ushort reg56;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_mrlast_thd : 5;      //auto mhe2mi_rlast threshold
            ushort reg_mhe_marb_mwlast_thd : 5;      //auto mhe2mi_wlast threshold
            ushort reg57_dummy : 2;
            ushort reg_mhe_marb_burst_split : 2;     //0: disable; N: MIU Request IDLE “N” cycles with every last signal
            ushort reg_mhe_marb_mr_reburst : 2;      //0: 4 req, 1: 8req, 2: 12req, 3: 16req
        };
        ushort reg57;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_0_low : 16;
        };
        ushort reg58;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_0_high : 12;  //miu write protection, miu upper bound 0  (BSP obuf)
        };
        ushort reg59;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_0_low : 16;
        };
        ushort reg5a;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_0_high : 12;  //miu write protection, miu lower bound 0
            ushort reg5b_dummy : 1;
            ushort reg_mhe_marb_miu_bound_en_0 : 1;  //miu write protection, miu bound enable for write port 0
        };
        ushort reg5b;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_1_low : 16;
        };
        ushort reg5c;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_1_high : 12;  //miu write protection, miu upper bound 1 (MC obuf)
        };
        ushort reg5d;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_1_low : 16;
        };
        ushort reg5e;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_1_high : 12;  //miu write protection, miu lower bound 1
            ushort reg5f_dummy : 1;
            ushort reg_mhe_marb_miu_bound_en_1 : 1;  //miu write protection, miu bound enable for write port 1
        };
        ushort reg5f;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_2_low : 16;
        };
        ushort reg60;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_2_high : 12;  //miu write protection, miu upper bound 2 (MV obuf)
        };
        ushort reg61;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_2_low : 16;
        };
        ushort reg62;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_2_high : 12;  //miu write protection, miu lower bound 2
            ushort reg63_dummy : 1;
            ushort reg_mhe_marb_miu_bound_en_2 : 1;  //miu write protection, miu bound enable for write port 2
        };
        ushort reg63;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_3_low : 16;
        };
        ushort reg64;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_ubound_3_high : 12;  //miu write protection, miu upper bound 3 (GN)
        };
        ushort reg65;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_3_low : 16;
        };
        ushort reg66;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_lbound_3_high : 12;  //miu write protection, miu lower bound 3
            ushort reg67_dummy : 1;
            ushort reg_mhe_marb_miu_bound_en_3 : 1;  //miu write protection, miu bound enable for write port 3
            ushort reg_mhe_marb_miu_off : 1;         //miu write protection, miu off
            ushort reg_mhe_marb_miu_bound_err : 1;   //[r] miu write protection, miu bound error status for write port 0 ~ 3
        };
        ushort reg67;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_eimi_block : 1;      //miu emi/imi block, 0: disable;
            ushort reg_mhe_marb_lbwd_mode : 1;       //low bandwidth mode, 0: disable
            ushort reg_mhe_marb_imi_burst_thd : 5;   //imi write burst bunch up threshold
            ushort reg_mhe_marb_imi_timeout : 3;     //imi write burst timeout
            ushort reg_mhe_marb_imilast_thd : 4;     //auto mhe2imi_last threshold
        };
        ushort reg68;
    };
    union
    {
        struct
        {
            ushort reg_mhe_pat_gen_init : 16;        //pattern generation initial value
        };
        ushort reg69;
    };
    union
    {
        struct
        {
            ushort reg6a_dummy : 13;
            ushort reg_mfe_pat_gen_en : 1;           //enable pattern generation
        };
        ushort reg6a;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_imi_sadr_low : 16;
        };
        ushort reg6b;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_imi_sadr_high : 13;  //imi base address for low bandwdith mode
            ushort reg_mhe_marb_rimi_force : 1;      //'b1:force all read path to IMI   'b0:Normal
            ushort reg_mhe_marb_imi_cache_size : 2;  //imi cache size (0: 64kB, 1:32kB, 2:16kB, 3:8kB)
        };
        ushort reg6c;
    };
    union
    {
        struct
        {
            ushort reg_mhe_marb_imipriority_thd : 4; //hardware mhe2imi_priority threshold
            ushort reg_mhe_marb_imipriority_sw : 2;  //mhe2imi_priority software programmable
        };
        ushort reg6d;
    };
    union
    {
        struct
        {
            ushort reg_mhe_debug_mode : 7;           //debug mode
        };
        ushort reg70;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_mode : 8;             //[4]: crc enable, [3:0]: crc in select d0~d7(rp0~rp7), d8~d15(wp0~wp7)
            ushort reg73_dummy : 6;
            ushort reg_mhe_crc_result_sel : 2;       //'h0: [63:0]  'h1: [127:64]  'h2: [191:128]  'h3: [255:192]
        };
        ushort reg73;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_rd0 : 16;
        };
        ushort reg74;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_rd1 : 16;
        };
        ushort reg75;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_rd2 : 16;
        };
        ushort reg76;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_rd3 : 16;
        };
        ushort reg77;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_rd20 : 16;
        };
        ushort reg78;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_rd21 : 16;
        };
        ushort reg79;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_rd22 : 16;
        };
        ushort reg7a;
    };
    union
    {
        struct
        {
            ushort reg_mhe_crc_result_rd23 : 16;
        };
        ushort reg7b;
    };
    union
    {
        struct
        {
            ushort reg_mhe_rsv7c : 16;           //reserved registers
        };
        ushort reg7c;
    };
    union
    {
        struct
        {
            ushort reg_mhe_rsv7d : 16;           //reserved registers
        };
        ushort reg7d;
    };
    union
    {
        struct
        {
            ushort reg_mhe_rsv7e : 16;           //reserved registers
        };
        ushort reg7e;
    };
    union
    {
        struct
        {
            ushort reg_mhe_rsv7f : 16;           //reserved registers
        };
        ushort reg7f;
    };
} MHERegTable;

///////////////////////// HEV Bank1 ////////////////////////
// This map to HW excel file hev_register_definition_bank1
//RIU_BANK1_ADDR or RIU_BANK1_ADDR_CORE1
typedef struct _hev1_reg_
{
    // [MBR]
    // CTB Allocate Bit & LUT
    union
    {
        struct
        {
            ushort reg_mbr_bit_per_weight_low : 16;
        };
        ushort reg00;
    };
    union
    {
        struct
        {
            ushort reg_mbr_bit_per_weight_high : 6;              //Used in CTB allocate bit Fractional:10, Int: 12
            ushort reg_mbr_smooth_window : 4;                    //smooth window in CTB bit allocation
            ushort reg_mbr_smooth_window_on : 1;                 //turn on smooth window
            ushort reg_mbr_const_qp_en : 1;                      //Use constant qp value in LUT entry 7
            ushort reg_mbr_lut_entry_clip_range : 4;             //Clip LUT entry in +- clip_range of previous LUT entry
        };
        ushort reg01;
    };
    // Second Stage
    union
    {
        struct
        {
            ushort reg_mbr_ss_alpha : 5;                         //alpha blending used in the second stage
            ushort reg_mbr_ss_turn_off_perceptual : 1;           //1: tie stage one output as 0
            ushort reg_mbr_ss_sw_read_weighting : 8;             //Not read weighting from MIU, but use this value
            ushort reg_mbr_ss_turn_off_read_weighting : 1;       //1: tie miu read value as register value
        };
        ushort reg02;
    };
    union
    {
        struct
        {
            ushort reg_mbr_ss_bit_weighting_offset : 8;          //bit weight offset in the second stage
        };
        ushort reg03;
    };
    // Texture Weight
    union
    {
        struct
        {
            ushort reg_mbr_tc_text_weight_type : 2;              //texture complexity weight type
            ushort reg_mbr_tc_clip_thr : 6;                      //clip threshold in texture complexity
            ushort reg_mbr_tc_offset : 8;                        //signed value of texture complexity offset
        };
        ushort reg04;
    };
    // Pixel Color Weight
    union
    {
        struct
        {
            ushort reg_mbr_pw_y_max : 8;                         //Protected color Y Max
            ushort reg_mbr_pw_y_min : 8;                         //Protected color Y Min
        };
        ushort reg05;
    };
    union
    {
        struct
        {
            ushort reg_mbr_pw_cb_max : 8;                        //Protected color Cb Max
            ushort reg_mbr_pw_cb_min : 8;                        //Protected color Cb Min
        };
        ushort reg06;
    };
    union
    {
        struct
        {
            ushort reg_mbr_pw_cr_max : 8;                        //Protected color Cr Max
            ushort reg_mbr_pw_cr_min : 8;                        //Protected color Cr Min
        };
        ushort reg07;
    };
    union
    {
        struct
        {
            ushort reg_mbr_pw_cb_plus_cr_min : 9;                //Protected color CB_PLUS_CR_MIN
            ushort reg_mbr_pw_pc_dead_zone : 7;                  //Protected color pc dead zone
        };
        ushort reg08;
    };
    union
    {
        struct
        {
            ushort reg_mbr_pw_cr_offset : 8;                     //Protected color CR offset
            ushort reg_mbr_pw_cr_offset_on : 1;                  //1: Use protected color CR offset
            ushort reg_mbr_pw_pc_level_shift : 4;                //Protected color level shift in quantization
        };
        ushort reg09;
    };
    union
    {
        struct
        {
            ushort reg_mbr_pw_cb_cr_offset : 12;                 //Protected color Cb Cr offset
        };
        ushort reg0a;
    };
    // Bit Weight Fusion
    union
    {
        struct
        {
            ushort reg_mbr_sm_bit_weight_offset : 8;             //Bit weight offset in bit weight fusion
            ushort reg_mbr_sm_on : 1;                            //1: Turn on smoothing in bit weight fusion
            ushort reg_mbr_sm_text_alpha : 5;                    //Alpha blending used in bit weight fusion
            ushort reg_mbr_sm_write_turn_off : 1;                //1: Turn off write weighting to MIU
        };
        ushort reg0b;
    };
    // ROI
    union
    {
        struct
        {
            ushort reg_mbr_sm_clip : 8;                          //Clip "current sample" in bit weight fusion
            ushort reg_mbr_lut_roi_on : 1;                       //1: Turn on ROI
            ushort reg_txip_ctb_force_on : 1;                    //1: Turn on txip force (get data from MBR)
        };
        ushort reg0c;
    };
    // CTB weight frame buffer
    union
    {
        struct
        {
            ushort reg0e_dummy : 4;
            ushort reg_mbr_gn_write_st_addr_low : 12;
        };
        ushort reg0e;
    };
    union
    {
        struct
        {
            ushort reg_mbr_gn_write_st_addr_high : 16;       //CTB_weight write out start address (byte)  16 byte aligned
        };
        ushort reg0f;
    };
    union
    {
        struct
        {
            ushort reg10_dummy : 4;
            ushort reg_mbr_gn_read_st_addr_low : 12;
        };
        ushort reg10;
    };
    union
    {
        struct
        {
            ushort reg_mbr_gn_read_st_addr_high : 16;        //CTB_weight read from start address (byte)  16 byte aligned
        };
        ushort reg11;
    };
    union
    {
        struct
        {
            ushort reg12_dummy : 4;
            ushort reg_mbr_lut_st_addr_low : 12;
        };
        ushort reg12;
    };
    union
    {
        struct
        {
            ushort reg_mbr_lut_st_addr_high : 16;            //LUT read from start address (byte)  16 byte aligned
        };
        ushort reg13;
    };
    union
    {
        struct
        {
            ushort reg14_dummy : 4;
            ushort reg_mbr_gn_read_map_st_addr_low : 12;
        };
        ushort reg14;
    };
    union
    {
        struct
        {
            ushort reg_mbr_gn_read_map_st_addr_high : 16;    //LUT delta map read from start address (byte)  16 byte aligned
        };
        ushort reg15;
    };
    // HIST read
    union
    {
        struct
        {
            ushort reg_mbr_hist_0 : 16;                      //[r] LUT entry histogram entry 0
        };
        ushort reg16;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_1 : 16;                      //[r] LUT entry histogram entry 1
        };
        ushort reg17;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_2 : 16;                      //[r] LUT entry histogram entry 2
        };
        ushort reg18;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_3 : 16;                      //[r] LUT entry histogram entry 3
        };
        ushort reg19;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_4 : 16;                      //[r] LUT entry histogram entry 4
        };
        ushort reg1a;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_5 : 16;                      //[r] LUT entry histogram entry 5
        };
        ushort reg1b;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_6 : 16;                      //[r] LUT entry histogram entry 6
        };
        ushort reg1c;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_7 : 16;                      //[r] LUT entry histogram entry 7
        };
        ushort reg1d;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_8 : 16;                      //[r] LUT entry histogram entry 8
        };
        ushort reg1e;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_9 : 16;                      //[r] LUT entry histogram entry 9
        };
        ushort reg1f;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_a : 16;                      //[r] LUT entry histogram entry a
        };
        ushort reg20;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_b : 16;                      //[r] LUT entry histogram entry b
        };
        ushort reg21;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_c : 16;                      //[r] LUT entry histogram entry c
        };
        ushort reg22;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_d : 16;                      //[r] LUT entry histogram entry d
        };
        ushort reg23;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_e : 16;                      //[r] LUT entry histogram entry e
        };
        ushort reg24;
    };
    // Statistic Accumulation
    union
    {
        struct
        {
            ushort reg_mbr_bit_weighting_out_accum_low : 16;
        };
        ushort reg26;
    };
    union
    {
        struct
        {
            ushort reg_mbr_bit_weighting_out_accum_high : 8; //[r] Frist stage bit_weighting out accumulation (CTB size, if stage 1 is turned off)
            ushort reg_mbr_ctb_bit_weighting_accum_low : 8;
        };
        ushort reg27;
    };
    union
    {
        struct
        {
            ushort reg_mbr_ctb_bit_weighting_accum_high : 16; //[r] Second stage bit_weighting out accumulation
        };
        ushort reg28;
    };
    union
    {
        struct
        {
            ushort reg_mbr_tc_accum_low : 16;
        };
        ushort reg29;
    };
    union
    {
        struct
        {
            ushort reg_mbr_tc_accum_high : 8;                //[r] Texture weight accumulation
            ushort reg_mbr_pc_pixel_count_accum_low : 8;
        };
        ushort reg2a;
    };
    union
    {
        struct
        {
            ushort reg_mbr_pc_pixel_count_accum_high : 16;   //[r] Pixel color weight accumulation
        };
        ushort reg2b;
    };
    // HIST of pc and tw read
    union
    {
        struct
        {
            ushort reg_hev_cmdq_irq_mask : 8;                //0/1: irq not-mask/mask
            ushort reg_hev_cmdq_irq_force : 8;
        };
        ushort reg2c;
    };
    union
    {
        struct
        {
            ushort reg_hev_cmdq_irq_clr0 : 1;                //0/1: not clear interrupt/clear interrupt 0 (write one clear)
            ushort reg_hev_cmdq_irq_clr1 : 1;                //0/1: not clear interrupt/clear interrupt 1 (write one clear)
            ushort reg_hev_cmdq_irq_clr2 : 1;                //0/1: not clear interrupt/clear interrupt 2 (write one clear)
            ushort reg_hev_cmdq_irq_clr3 : 1;                //0/1: not clear interrupt/clear interrupt 3 (write one clear)
            ushort reg_hev_cmdq_irq_clr4 : 1;                //0/1: not clear interrupt/clear interrupt 4 (write one clear)
            ushort reg_hev_cmdq_irq_clr5 : 1;                //0/1: not clear interrupt/clear interrupt 5 (write one clear)
            ushort reg_hev_cmdq_irq_clr6 : 1;                //0/1: not clear interrupt/clear interrupt 6 (write one clear)
            ushort reg_hev_cmdq_irq_clr7 : 1;                //0/1: not clear interrupt/clear interrupt 7 (write one clear)
        };
        ushort reg2d;
    };
    union
    {
        struct
        {
            ushort reg_hev_cmdq_st_irq_cpu : 8;              // [r]
            ushort reg_hev_cmdq_st_irq_ip : 8;               // [r]
        };
        ushort reg2e;
    };
    // [MFCodec]
    // fbenc/dec
    union
    {
        struct
        {
            ushort reg_tagram_inv : 1;                       //[w] FW force invalidate
            ushort reg_en_fb_enc : 1;                        //enable frame buffer encode
            ushort reg_en_fb_dec : 1;                        //enable frame buffer decode
            ushort reg_en_bw_pack : 1;                       //enable byte-write packer
            ushort reg_vp9_mode : 1;                         //VP9 enable
            ushort reg_bitlen_prefetch_en : 1;               //Bit-length DMA prefetch enable
            ushort reg_bypass_cache_en : 1;                  //1: bypass 0: cache
            ushort reg_fbcodec_sw_rst : 1;                   //Frame buffer codec SW reset
            ushort reg_ctb_sz : 3;                           //Coding tree Block size and PCM mode is included 0,1 reserved 2: 16x16, 3 32x32, 4: 64x64, 5~: reserved HM: getMaxCUWidth()
            ushort reg_mfc_prefh_en : 1;                     //enable fbdec bl pre-fetch
            ushort reg_fbc_gated_en : 1;                     //enable fbc gated clk
            ushort reg_fbc_gated_slice_en : 1;               //enable fbc slice gated clk
            ushort reg_fbdec_sw_inval : 1;                   //sw cache invalid clear
            ushort reg_i_slice : 1;                          //I or p frame
        };
        ushort reg30;
    };
    union
    {
        struct
        {
            ushort reg_fbenc_ctrl_bus_low : 16;
        };
        ushort reg31;
    };
    union
    {
        struct
        {
            ushort reg_fbenc_ctrl_bus_high : 16;             //Frame buffer encoder mode(reg_ev_fenc_mode)
        };
        ushort reg32;
    };
    union
    {
        struct
        {
            ushort reg33_dummy : 5;
            ushort reg_fb_pitch : 9;                         //Frame buffer pitch, unit 32 bytes
        };
        ushort reg33;
    };
    union
    {
        struct
        {
            ushort reg34_dummy : 5;
            ushort reg_fb_bitlen_pitch : 9;                  //Frame buffer codec bit-len pitch, unit 32 bytes
        };
        ushort reg34;
    };
    union
    {
        struct
        {
            ushort reg35_dummy : 9;
            ushort reg_fb_bitlen_sz_low : 7;
        };
        ushort reg35;
    };
    union
    {
        struct
        {
            ushort reg_fb_bitlen_sz_high : 5;                //Frame buffer codec bit-len size and 512bytes alignment. Size = Width_in_32B * Height_in_16 * 1.5, unit 512bytes
        };
        ushort reg36;
    };
    union
    {
        struct
        {
            ushort reg37_dummy : 9;
            ushort reg_cur_fb_bitlen_addr_low : 7;
        };
        ushort reg37;
    };
    // Frame setting of bit length address
    union
    {
        struct
        {
            ushort reg_cur_fb_bitlen_addr_high : 16;     //Frame buffer codec bit-len current address and 512bytes alignment.
        };
        ushort reg38;
    };
    union
    {
        struct
        {
            ushort reg39_dummy : 9;
            ushort reg_fb_bitlen_base_low : 7;
        };
        ushort reg39;
    };
    union
    {
        struct
        {
            ushort reg_fb_bitlen_base_high : 16;     //Frame buffer codec bit-len base address and 512bytes alignment.
        };
        ushort reg3a;
    };
    // Huffman table
    union
    {
        struct
        {
            ushort reg_cw_len2_mask : 12;            //Codeword length mask for codeword length is 2
        };
        ushort reg3b;
    };
    union
    {
        struct
        {
            ushort reg_cw_len3_mask : 12;            //Codeword length mask for codeword length is 3
        };
        ushort reg3c;
    };
    union
    {
        struct
        {
            ushort reg_cw_len4_mask : 12;            //Codeword length mask for codeword length is 4
        };
        ushort reg3d;
    };
    union
    {
        struct
        {
            ushort reg_cw_len5_mask : 12;            //Codeword length mask for codeword length is 5
        };
        ushort reg3e;
    };
    union
    {
        struct
        {
            ushort reg_cw_base : 12;                 //Codeword base
        };
        ushort reg3f;
    };
    union
    {
        struct
        {
            ushort reg_cw_max : 12;                  //Codeword maximum number
        };
        ushort reg40;
    };
    union
    {
        struct
        {
            ushort reg_symb_base : 5;                //Symbol base
            ushort reg_symb_max : 5;                 //Symbol max
        };
        ushort reg41;
    };
    union
    {
        struct
        {
            ushort reg_huf_tab_prg_addr : 4;         //Huffman table address
            ushort reg_huf_tab_prg_go : 1;           //Huffman table programming, 1T pulse
            ushort reg_huf_tab_prg_idx : 2;          //Huffman table index,0: CW = 2, 1: CW = 3, 2: CW = 4, 3: CW = 5
            ushort reg_huf_tab_prg_rw : 1;           //Huffman table read/write, 0: read, 1: write
        };
        ushort reg42;
    };
    // Cache
    union
    {
        struct
        {
            ushort reg_l_cache_burst_cnt : 3;        //CPX luma cache burst count, unit is 8
            ushort reg_l_cache_flush_cnt : 3;        //CPX luma cache flush count, unit is 4
            ushort reg_l_cache_flush_timeout : 3;    //CPX luma cache flush timeout count, unit is 4
        };
        ushort reg43;
    };
    union
    {
        struct
        {
            ushort reg_c_cache_burst_cnt : 3;        //MC LSB chroma cache burst count, unit is 8
            ushort reg_c_cache_flush_cnt : 3;        //MC LSB chroma cache flush count, unit is 4
            ushort reg_c_cache_flush_timeout : 3;    //MC LSB chroma cache flush timeout count, unit is 4
        };
        ushort reg44;
    };
    // Rdo
    union
    {
        struct
        {
            ushort reg_chroma_rdo_th1 : 16;          //Chroma byte count difference threshold1, unit 32bytes
        };
        ushort reg45;
    };
    union
    {
        struct
        {
            ushort reg_chroma_rdo_th2 : 16;          //Chroma byte count difference threshold2, unit 32bytes
        };
        ushort reg46;
    };
    union
    {
        struct
        {
            ushort reg_chroma_rdo_t0 : 16;           //Chroma initial offset, unit 32bytes
        };
        ushort reg47;
    };
    union
    {
        struct
        {
            ushort reg_chroma_rdo_delta : 12;        //Chroma delta , format 7.5
            ushort reg_chroma_en_rdo : 1;            //Chroma lossy mode enable
            ushort reg_chroma_lossy_debug_en : 1;    //Chroma lossy debug mode enable
            ushort reg_chroma_sw_lossy_mode : 1;     //Chroma SW lossy mode
            ushort reg_chroma_sw_force_lossy : 1;    //Chroma SW force lossy enable
        };
        ushort reg48;
    };
    union
    {
        struct
        {
            ushort reg_chroma_lossy_debug_pattern_low : 16;
        };
        ushort reg49;
    };
    union
    {
        struct
        {
            ushort reg_chroma_lossy_debug_pattern_high : 16; //Chroma lossy debug pattern
        };
        ushort reg4a;
    };
    union
    {
        struct
        {
            ushort reg_luma_rdo_th1 : 16;        //Luma byte count difference threshold1, unit 32bytes
        };
        ushort reg4b;
    };
    union
    {
        struct
        {
            ushort reg_luma_rdo_th2 : 16;        //Luma byte count difference threshold2, unit 32bytes
        };
        ushort reg4c;
    };
    union
    {
        struct
        {
            ushort reg_luma_rdo_t0 : 16;         //Luma initial offset, unit 32bytes
        };
        ushort reg4d;
    };
    union
    {
        struct
        {
            ushort reg_luma_rdo_delta : 12;      //Luma delta , format 7.5
            ushort reg_luma_en_rdo : 1;          //Luma lossy mode enable
            ushort reg_luma_lossy_debug_en : 1;  //Luma lossy debug mode enable
            ushort reg_luma_sw_lossy_mode : 1;   //Luma SW lossy mode
            ushort reg_luma_sw_force_lossy : 1;  //Luma SW force lossy enable
        };
        ushort reg4e;
    };
    union
    {
        struct
        {
            ushort reg_luma_lossy_debug_pattern_low : 16;
        };
        ushort reg4f;
    };
    union
    {
        struct
        {
            ushort reg_luma_lossy_debug_pattern_high : 16;   //Luma lossy debug pattern
        };
        ushort reg50;
    };
    // Lossy compress mode
    union
    {
        struct
        {
            ushort reg_lossy_comp_mode : 2; //lossy bits compensated mode select 0 : fix pattern 1 : checkboard pattern 2 : random noise dither 3 : random noise dither (2bit lossy only compensate 2'b01 or 2'10)
            ushort reg_lossy_2bit_pat : 2;               //lossy 2bit compensated pattern
            ushort reg_lossy_1bit_pat : 1;               //lossy 1bit compensated pattern
        };
        ushort reg51;
    };
    // Dual core
    union
    {
        struct
        {
            ushort reg_mhe_dual_row_dbfprf : 8;          //dual core dbf faster than prfh ctb-row cnt
            ushort reg_mhe_dual_ctb_dbfprf : 8;          //dual core dbf faster than prfh ctb cnt
        };
        ushort reg53;
    };
    union
    {
        struct
        {
            ushort reg54_dummy : 1;
            ushort reg_mhe_dual_core : 1;                //1:dual core, 0:single core
            ushort reg54_dummy1 : 1;
            ushort reg_mhe_dual_strm_id : 3;             //stream index
            ushort reg_mhe_dual_bs0_rstz : 1;            //software reset 0: Reset 1: Not reset
            ushort reg_mhe_dual_bs1_rstz : 1;            //software reset 0: Reset 1: Not reset
        };
        ushort reg54;
    };
    // Read
    union
    {
        struct
        {
            ushort reg_cur_cw_base : 12;                 //[r] Current codeword base
        };
        ushort reg55;
    };
    union
    {
        struct
        {
            ushort reg_cur_cw_max : 12;                  //[r] Current codeword maximum
        };
        ushort reg56;
    };
    union
    {
        struct
        {
            ushort reg_cur_symb_base : 5;                //[r] Current symbol base
            ushort reg_cur_symb_max : 5;                 //[r] Current symbol maximum
        };
        ushort reg57;
    };
    union
    {
        struct
        {
            ushort reg_chroma_rdo_bc_low : 16;
        };
        ushort reg58;
    };
    union
    {
        struct
        {
            ushort reg_chroma_rdo_bc_high : 8;           //[r] Chroma frame byte count
        };
        ushort reg59;
    };
    union
    {
        struct
        {
            ushort reg_chroma_rdo_blkcnt_low : 16;
        };
        ushort reg5a;
    };
    union
    {
        struct
        {
            ushort reg_chroma_rdo_blkcnt_high : 1;       //[r] Chroma frame 32x4 block count
        };
        ushort reg5b;
    };
    union
    {
        struct
        {
            ushort reg_chroma_1bit_loss_blkcnt : 16;     //[r] Chroma 1-bit loss 32x4 block count
        };
        ushort reg5c;
    };
    union
    {
        struct
        {
            ushort reg_chroma_2bit_loss_blkcnt : 16;     //[r] Chroma 2-bit loss 32x4 block count
        };
        ushort reg5d;
    };
    union
    {
        struct
        {
            ushort reg_luma_rdo_bc_low : 16;
        };
        ushort reg5e;
    };
    union
    {
        struct
        {
            ushort reg_luma_rdo_bc_high : 8;             //[r] Luma frame byte count
        };
        ushort reg5f;
    };
    union
    {
        struct
        {
            ushort reg_luma_rdo_blkcnt_low : 16;
        };
        ushort reg60;
    };
    union
    {
        struct
        {
            ushort reg_luma_rdo_blkcnt_high : 1;         //[r] Luma frame 32x4 block count
        };
        ushort reg61;
    };
    union
    {
        struct
        {
            ushort reg_luma_1bit_loss_blkcnt : 16;       //[r] Luma 1-bit loss 32x4 block count
        };
        ushort reg62;
    };
    union
    {
        struct
        {
            ushort reg_luma_2bit_loss_blkcnt : 16;       //[r] Luma 2-bit loss 32x4 block count
        };
        ushort reg63;
    };
    union
    {
        struct
        {
            ushort reg_fbcodec_ctb16_mode : 8;           //[r] Frame buffer Codec version, 0: version 1.0, 1: version 2.0, 2: version 2.5, 3: version 3.0
            ushort reg_fbdec_che_inval_done : 6;         //[r] Frame buffer decoder cache invalid done signals
            ushort reg_fbcodec_sw_rst_finish : 1;        //[r] Frame buffer codec SW reset finish
            ushort reg_fbc_clk_act : 1;                  //[r] fbc clk act
        };
        ushort reg64;
    };
    union
    {
        struct
        {
            ushort reg_luma_first_1b_loss_blkidx : 16;   //[r] Luma 1-bit loss block index
        };
        ushort reg65;
    };
    union
    {
        struct
        {
            ushort reg_luma_first_2b_loss_blkidx : 16;   //[r] Luma 2-bit loss block index
        };
        ushort reg66;
    };
    union
    {
        struct
        {
            ushort reg_chroma_first_1b_loss_blkidx : 16; //[r] Chroma 1-bit loss block index
        };
        ushort reg67;
    };
    union
    {
        struct
        {
            ushort reg_chroma_first_2b_loss_blkidx : 16; //[r] Chroma 2-bit loss block index
        };
        ushort reg68;
    };
    // JPD handshake
    union
    {
        struct
        {
            ushort reg_mhe_jpd_delta : 8;                //8-bit, delta between JPD counter and MHE counter
            ushort reg_mhe_jpd_hsk_en : 1;
            ushort reg_mhe_jpd_format : 1;               //0:420, 1:422
            ushort reg_mhe_jpd_hsk_frame_idx : 2;        //JPD handshake frame index
        };
        ushort reg69;
    };
    union
    {
        struct
        {
            ushort reg_hev_ec_bsp_thd_size_low : 16;
        };
        ushort reg6a;
    };
    // Bitstream Threshold
    union
    {
        struct
        {
            ushort reg_hev_ec_bsp_thd_size_high : 13;    //Bitstream size threshold (Byte)
            ushort reg6b_dummy : 2;
            ushort reg_hev_ec_bsp_thd_en : 1;            //Bitstream size threshold enable
        };
        ushort reg6b;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_bsp_thd_ctbx : 8;       //CTB X while reaching threshold
            ushort reg_ro_hev_ec_bsp_thd_ctby : 8;       //CTB Y while reaching threshold
        };
        ushort reg6c;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_bsp_bit_cnt_low : 16;
        };
        ushort reg6d;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_bsp_bit_cnt_high : 16;  //Bitstream size (bit)
        };
        ushort reg6e;
    };
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_bspobuf_wptr_fix_low : 16;
        };
        ushort reg6f;
    };
    // BSPobuf write pointer
    union
    {
        struct
        {
            ushort reg_ro_hev_ec_bspobuf_wptr_fix_high : 12; //BSPobuf current write address
            ushort reg_w1c_hev_ec_bspobuf_wptr_lat : 1;      //BSPobuf write address latch signal
        };
        ushort reg70;
    };

    // NOTE: These are NOT registers yet !
    //Int reg_ctb_width;
    //Int reg_ctb_height;
} HEV1RegTable;

///////////////////////// HEV Bank2 ////////////////////////
// This map to HW excel file hev_register_definition_bank2
//RIU_BANK2_ADDR or RIU_BANK2_ADDR_CORE1
typedef struct _hev2_reg_
{
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_0 : 16;   //[r] Protected color weighting histogram 0
        };
        ushort reg00;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_1 : 16;   //[r] Protected color weighting histogram 1
        };
        ushort reg01;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_2 : 16;   //[r] Protected color weighting histogram 2
        };
        ushort reg02;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_3 : 16;   //[r] Protected color weighting histogram 3
        };
        ushort reg03;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_4 : 16;   //[r] Protected color weighting histogram 4
        };
        ushort reg04;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_5 : 16;   //[r] Protected color weighting histogram 5
        };
        ushort reg05;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_6 : 16;   //[r] Protected color weighting histogram 6
        };
        ushort reg06;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_7 : 16;   //[r] Protected color weighting histogram 7
        };
        ushort reg07;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_8 : 16;   //[r] Protected color weighting histogram 8
        };
        ushort reg08;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_9 : 16;   //[r] Protected color weighting histogram 9
        };
        ushort reg09;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_a : 16;   //[r] Protected color weighting histogram 10
        };
        ushort reg0a;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_b : 16;   //[r] Protected color weighting histogram 11
        };
        ushort reg0b;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_c : 16;   //[r] Protected color weighting histogram 12
        };
        ushort reg0c;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_d : 16;   //[r] Protected color weighting histogram 13
        };
        ushort reg0d;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_e : 16;   //[r] Protected color weighting histogram 14
        };
        ushort reg0e;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_f : 16;   //[r] Protected color weighting histogram 15
        };
        ushort reg0f;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_10 : 16;  //[r] Protected color weighting histogram 16
        };
        ushort reg10;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_11 : 16;  //[r] Protected color weighting histogram 17
        };
        ushort reg11;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_12 : 16;  //[r] Protected color weighting histogram 18
        };
        ushort reg12;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_13 : 16;  //[r] Protected color weighting histogram 19
        };
        ushort reg13;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_14 : 16;  //[r] Protected color weighting histogram 20
        };
        ushort reg14;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_15 : 16;  //[r] Protected color weighting histogram 21
        };
        ushort reg15;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_16 : 16;  //[r] Protected color weighting histogram 22
        };
        ushort reg16;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_17 : 16;  //[r] Protected color weighting histogram 23
        };
        ushort reg17;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_18 : 16;  //[r] Protected color weighting histogram 24
        };
        ushort reg18;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_19 : 16;  //[r] Protected color weighting histogram 25
        };
        ushort reg19;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_1a : 16;  //[r] Protected color weighting histogram 26
        };
        ushort reg1a;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_1b : 16;  //[r] Protected color weighting histogram 27
        };
        ushort reg1b;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_1c : 16;  //[r] Protected color weighting histogram 28
        };
        ushort reg1c;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_1d : 16;  //[r] Protected color weighting histogram 29
        };
        ushort reg1d;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_1e : 16;  //[r] Protected color weighting histogram 30
        };
        ushort reg1e;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_pc_1f : 16;  //[r] Protected color weighting histogram 31
        };
        ushort reg1f;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_0 : 16;   //[r] Texture weighting histogram 0
        };
        ushort reg20;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_1 : 16;   //[r] Texture weighting histogram 1
        };
        ushort reg21;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_2 : 16;   //[r] Texture weighting histogram 2
        };
        ushort reg22;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_3 : 16;   //[r] Texture weighting histogram 3
        };
        ushort reg23;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_4 : 16;   //[r] Texture weighting histogram 4
        };
        ushort reg24;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_5 : 16;   //[r] Texture weighting histogram 5
        };
        ushort reg25;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_6 : 16;   //[r] Texture weighting histogram 6
        };
        ushort reg26;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_7 : 16;   //[r] Texture weighting histogram 7
        };
        ushort reg27;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_8 : 16;   //[r] Texture weighting histogram 8
        };
        ushort reg28;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_9 : 16;   //[r] Texture weighting histogram 9
        };
        ushort reg29;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_a : 16;   //[r] Texture weighting histogram 10
        };
        ushort reg2a;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_b : 16;   //[r] Texture weighting histogram 11
        };
        ushort reg2b;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_c : 16;   //[r] Texture weighting histogram 12
        };
        ushort reg2c;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_d : 16;   //[r] Texture weighting histogram 13
        };
        ushort reg2d;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_e : 16;   //[r] Texture weighting histogram 14
        };
        ushort reg2e;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_f : 16;   //[r] Texture weighting histogram 15
        };
        ushort reg2f;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_10 : 16;  //[r] Texture weighting histogram 16
        };
        ushort reg30;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_11 : 16;  //[r] Texture weighting histogram 17
        };
        ushort reg31;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_12 : 16;  //[r] Texture weighting histogram 18
        };
        ushort reg32;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_13 : 16;  //[r] Texture weighting histogram 19
        };
        ushort reg33;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_14 : 16;  //[r] Texture weighting histogram 20
        };
        ushort reg34;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_15 : 16;  //[r] Texture weighting histogram 21
        };
        ushort reg35;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_16 : 16;  //[r] Texture weighting histogram 22
        };
        ushort reg36;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_17 : 16;  //[r] Texture weighting histogram 23
        };
        ushort reg37;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_18 : 16;  //[r] Texture weighting histogram 24
        };
        ushort reg38;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_19 : 16;  //[r] Texture weighting histogram 25
        };
        ushort reg39;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_1a : 16;  //[r] Texture weighting histogram 26
        };
        ushort reg3a;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_1b : 16;  //[r] Texture weighting histogram 27
        };
        ushort reg3b;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_1c : 16;  //[r] Texture weighting histogram 28
        };
        ushort reg3c;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_1d : 16;  //[r] Texture weighting histogram 29
        };
        ushort reg3d;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_1e : 16;  //[r] Texture weighting histogram 30
        };
        ushort reg3e;
    };
    union
    {
        struct
        {
            ushort reg_mbr_hist_tw_1f : 16;  //[r] Texture weighting histogram 31
        };
        ushort reg3f;
    };
    union
    {
        struct
        {
            ushort reg_hev_newme_h_search_max : 4;   //mvx integer search range, h2: -32 to +31, h6: -96 to +95
            ushort reg_hev_newme_v_search_max : 4;   //mvy integer search range, h1: -16 to +15, h3: -48 to +47
        };
        ushort reg5a;
    };
    union
    {
        struct
        {
            ushort reg_hev_newme_en : 1;             //enable newme
            ushort reg_hev_newme_mcc_imi_en : 1;     //enable newme mcc cache access imi
            ushort reg5c_dummy : 2;
            ushort reg_hev_newme_me_buf_size : 4;    //newme MELT buffer size (mb rows)
            ushort reg_hev_newme_chroma_buf_size : 8; //newme mcc cache  buffer size (KB)
        };
        ushort reg5c;
    };
    union
    {
        struct
        {
            ushort reg5e_dummy : 6;
            ushort reg_hev_newme_chroma_buf_adr_low : 10;
        };
        ushort reg5e;
    };
    union
    {
        struct
        {
            ushort reg_hev_newme_chroma_buf_adr_high : 16;   //base address in IMI for MC chroma cache
        };
        ushort reg5f;
    };
    // [low Bandwidth Mode]
    union
    {
        struct
        {
            ushort reg_hev_lbw_mode : 1;     // reference luma low bandwith mode enable
            ushort reg_hev_mcc_lbw_mode : 1; // reference chroma low bandwith mode enable
        };
        ushort reg60;
    };
    union
    {
        struct
        {
            ushort reg61_dummy : 5;
            ushort reg_hev_refy_imi_sadr_low : 11;
        };
        ushort reg61;
    };
    union
    {
        struct
        {
            ushort reg_hev_refy_imi_sadr_high : 16;  //reference luma imi start address, 32byte aligned
        };
        ushort reg62;
    };
    union
    {
        struct
        {
            ushort reg63_dummy : 5;
            ushort reg_hev_mcc_imi_sadr_low : 11;
        };
        ushort reg63;
    };
    union
    {
        struct
        {
            ushort reg_hev_mcc_imi_sadr_high : 16;       //reference chroma imi start address, 32byte aligned
        };
        ushort reg64;
    };
    // [Dump reg to DRAM]
    union
    {
        struct
        {
            ushort reg65_dummy : 4;
            ushort reg_hev_dump_reg_sadr_low : 12;
        };
        ushort reg65;
    };
    union
    {
        struct
        {
            ushort reg_hev_dump_reg_sadr_high : 16;  //write registers to DRAM start address, 16byte aligned
        };
        ushort reg66;
    };
    union
    {
        struct
        {
            ushort reg_hev_dump_reg_en : 1;          //write regisers to DRAM enable
        };
        ushort reg67;
    };
    // [Reserved]
    union
    {
        struct
        {
            ushort reg_hev_bk2_rsv78 : 16;           //reserved registers
        };
        ushort reg78;
    };
    union
    {
        struct
        {
            ushort reg_hev_bk2_rsv79 : 16;           //reserved registers
        };
        ushort reg79;
    };
    union
    {
        struct
        {
            ushort reg_hev_bk2_rsv7a : 16;           //reserved registers
        };
        ushort reg7a;
    };
    union
    {
        struct
        {
            ushort reg_hev_bk2_rsv7b : 16;           //reserved registers
        };
        ushort reg7b;
    };
    union
    {
        struct
        {
            ushort reg_hev_bk2_rsv7c : 16;           //reserved registers
        };
        ushort reg7c;
    };
    union
    {
        struct
        {
            ushort reg_hev_bk2_rsv7d : 16;           //reserved registers
        };
        ushort reg7d;
    };
    union
    {
        struct
        {
            ushort reg_hev_bk2_rsv7e : 16;           //reserved registers
        };
        ushort reg7e;
    };
    union
    {
        struct
        {
            ushort reg_hev_bk2_rsv7f : 16;           //reserved registers
        };
        ushort reg7f;
    };
} HEV2RegTable;

#pragma pack(push)
#pragma pack(1)
typedef struct MheDumpReg_t
{
    ushort hev_bank0_reg16;   // CABAC total bin counts
    ushort hev_bank0_reg17;   // CABAC total bin counts
    ushort hev_bank0_reg18;   // CABAC total bin counts
    ushort hev_bank0_reg19;   // CABAC total bin counts
    ushort hev_bank1_reg6d;   // bits size:lo
    ushort hev_bank1_reg6e;   // bits size:hi
    ushort hev_bank0_reg1a;   // bspobuf write pointer (8 byte unit)
    ushort hev_bank0_reg1b;   // bspobuf write pointer (8 byte unit)

    ushort hev_bank0_reg1e;   //IRQ
    ushort dummy1[7];

    ushort hev_bank1_reg16[15];  // LUT entry histogram entry 0~14
    char dummy2;
    char dummy3;
    ushort hev_bank2_reg00[32];  // Protected color weighting histogram
    ushort hev_bank2_reg20[32];  // Texture weighting histogram

    ushort hev_bank1_reg26;      //Frist stage bit_weighting out accumulation (CTB size, if stage 1 is turned off)
    uchar  hev_bank1_reg27_low;
    uchar  hev_bank1_reg27_high;
    ushort hev_bank1_reg28;      //Second stage bit_weighting out accumulation

    uchar  hev_bank1_reg2a_high; //Pixel color weight accumulation
    ushort hev_bank1_reg2b;

    ushort  hev_bank1_reg29;     //Texture weight accumulation
    uchar hev_bank1_reg2a_low;
} mhe_dump_reg;
#pragma pack(pop)

typedef struct MheReg_t
{
    mhve_job    mjob;
    int   coded_framecnt;   /* Coded Picture Count */
    /* SW-coded data followed by mhe-output stream */
    void* coded_data;       /* Pointer to bits_coded[256], ready for FDC */
    int   coded_bits;       /* Equal to bits_count */
    uchar bits_coded[256];  /* Buffer for store SPS/PPS/User Data/Slice Header */
    int   bits_count;       /* Total bit counts of SPS/PPS/User Data/Slice Header (Max 2048Bits) */
    int   bits_delta;       /* Bit count difference of real bit count and slice header bit count */
    uint  irq_status;       /* Interrupt status */
    /* MIU address for mhe-hw */
    uint  outbs_addr;       /* Output Bitstream Buffer Address for MIU */
    int   outbs_size;       /* Output Bitstream Buffer Size for MIU */
    // (When low-bandwidth mode) In SRAM (Otherwise in DRAM)
    uint gn_mem;                              // TXIP intermediate data
    uint ppu_int_b;                           // PPU intermediate data
    // (Only needed when low-bandwidth mode) In SRAM
    uint imi_ref_y_buf;
    uint imi_ref_c_buf;
    // In DRAM
    uint ppu_int_a;                           // PPU intermediate data
    uint ppu_y_base_buf[2];                   // 0:reconstructed Y buffer, 1:reference Y buffer
    uint ppu_c_base_buf[2];                   // 0:reconstructed C buffer, 1:reference C buffer
    uint col_w_sadr_buf[2];
    /* Ticks for profiling */
    uint  enc_cycles;       /* Encode cycles */
    uint  enc_bitcnt;       /* Encoded bit count include pure bitstream and FDC data */
    uint  enc_sumpqs;       /* Last frame average QP */
    /* PMBR Histogram/Accumulation information */
    ushort pmbr_tc_hist[32];
    ushort pmbr_pc_hist[32];
    ushort pmbr_lut_hist[15];
    uint pmbr_tc_accum;

    /* dump register */
#ifdef ENABLE_DUMP_REG
    uint  dump_reg_vaddr;
#endif

    HEVRegTable hev_bank0;
    MHERegTable mhe_bank0;
    HEV1RegTable hev_bank1;
    HEV2RegTable hev_bank2;

    ///////////////////////// Other Bank ////////////////////////
    union
    {
        struct
        {
            ushort regClk_mhe_clock_setting: 2;
            ushort regClk_clock_source: 3;
        };
        ushort regClk;
    };
} mhe_reg;


#endif /*_MHE_REG_H_*/
