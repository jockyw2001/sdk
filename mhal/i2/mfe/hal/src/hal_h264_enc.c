
#include "hal_mfe_def.h"
#include "hal_h264_enc.h"

#define EXTENDED_SAR            255
#define ABS(x) (((x)>=0)?(x):-(x))

static int VuiWriteParam(bs_t* bs, vui_t* vui);

//------------------------------------------------------------------------------
//  Function    : WriteEbsp
//  Description :
//------------------------------------------------------------------------------
static void WriteEbsp(bs_t* bs, uchar byte)
{
    if (bs->ebsp_zeros == 2 && byte < 4)
    {
        /* Add emulation_prevention_three_byte (EP3) */
        *bs->curr++ = 0x03;
        bs->ebsp_zeros = 0;
        bs->bits_pos += 8;
    }

    if (byte == 0)
        bs->ebsp_zeros++;
    else
        bs->ebsp_zeros = 0;
    *bs->curr++ = byte;
}

//------------------------------------------------------------------------------
//  Function    : BsWrite
//  Description :
//------------------------------------------------------------------------------
static int BsWrite(bs_t* bs, int len, uint bit)
{
    if (bs->left_count > len)
    {
        bs->left_count -= len;
        bs->code_bits += bit << bs->left_count;
    }
    else /* Need flush data from code_bits to buffer */
    {
        int left = (16 - len + bs->left_count);
        uint pattern = (bs->code_bits << 16) + (bit << left);

        bs->left_count = left;
        bs->code_bits = pattern & 0xFFFF;

        WriteEbsp(bs, (uchar)(pattern>>24));
        WriteEbsp(bs, (uchar)(pattern>>16));
    }
    bs->bits_pos += len;
    return len;
}

//------------------------------------------------------------------------------
//  Function    : BsAlign
//  Description : Fill the free bit space to be byte alignment.
//------------------------------------------------------------------------------
static int BsAlign(bs_t* bs, int bit)
{
    int  size = bs->left_count&0x7;
    uint code = bit?(1<<size)-1:0;

    if (size > 0)
        BsWrite(bs, size, code);
    return size;
}

//------------------------------------------------------------------------------
//  Function    : BsWriteUE
//  Description : Write the unsigned Exp-Golomb code.
//------------------------------------------------------------------------------
static int BsWriteUE(bs_t* bs, uint val)
{
    int n, tmp;
    tmp = (val+1)/2;
    for (n=0; n<16 && tmp!=0; n++)
        tmp /= 2;

    return BsWrite(bs, 2*n+1, val+1);
}

//------------------------------------------------------------------------------
//  Function    : BsWriteSE
//  Description : Write the signed Exp-Golomb code.
//------------------------------------------------------------------------------
static int BsWriteSE(bs_t* bs, int val)
{
    int tmp = 1 - 2*val;
    int n, val_abs;

    val_abs = ABS(val);
    for (n=0; n<16 && val_abs!=0; n++)
        val_abs /= 2;

    if (tmp < 0)
        tmp = 2*val;

    val = tmp;

    return BsWrite(bs, 2*n+1, (uint)val);
}

//------------------------------------------------------------------------------
//  Function    : BsWriteU1
//  Description : Write unsigned integer using 1 bits.
//------------------------------------------------------------------------------
static int BsWriteU1(bs_t* bs, uint val)
{
    return BsWrite(bs, 1, val!=0);
}

//------------------------------------------------------------------------------
//  Function    : BsWrite32
//  Description : Write unsigned integer using 32 bits.
//------------------------------------------------------------------------------
static int BsWrite32(bs_t* bs, uint val)
{
    int len;
    len = BsWrite(bs, 16, (val>>16)&0xFFFF);
    len += BsWrite(bs, 16, val&0xFFFF);
    return len;
}

//------------------------------------------------------------------------------
//  Function    : BsTrail
//  Description : Write trailing bit (1) and some zero bit to form RBSP.
//------------------------------------------------------------------------------
static int BsTrail(bs_t* bs)
{
    int len;

    bs->left_count--;
    bs->code_bits += 1 << bs->left_count;

    if (!bs->left_count)
    {
        WriteEbsp(bs, (uchar)(bs->code_bits>>8));
        WriteEbsp(bs, (uchar)(bs->code_bits));
        bs->left_count = 16;
        bs->code_bits = 0;
    }
    bs->bits_pos += 8;
    bs->bits_pos &= ~0x7;

    len = 1 + (bs->left_count&0x7);

    bs->left_count &= ~0x7;

    return len;
}

//------------------------------------------------------------------------------
//  Function    : AvcReset
//  Description : This function reset bitstream structure.
//------------------------------------------------------------------------------
int AvcReset(bs_t* bs, uchar* buff, int len)
{
    bs->bits_pos = 0;
    bs->left_count = 16; /* It means flush buffer every 16 bits */
    bs->code_bits = 0;
    bs->curr = bs->buffer = buff;
    bs->length = len;
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : AvcFlush
//  Description : This function flush bitstream.
//------------------------------------------------------------------------------
int AvcFlush(bs_t* bs)
{
    while (bs->left_count < 16)
    {
        WriteEbsp(bs, (uchar)(bs->code_bits>>8));
        bs->left_count += 8;
        bs->code_bits <<= 8;
    }
    bs->left_count = 16;
    bs->code_bits = 0;
    return bs->bits_pos;
}

//------------------------------------------------------------------------------
//  Function    : AvcCount
//  Description : This function get current write bit counts.
//------------------------------------------------------------------------------
int AvcCount(bs_t* bs)
{
    return bs->bits_pos;
}

//------------------------------------------------------------------------------
//  Function    : AvcWriteNAL
//  Description : This function write Start Code and NALU header to buffer.
//------------------------------------------------------------------------------
int AvcWriteNAL(bs_t* bs, uchar nal)
{
    /* The first 4 byte is START_CODE */ // EROY TBD : Need Modify for 3GP/AVI container
    *bs->curr++ = 0;
    *bs->curr++ = 0;
    *bs->curr++ = 0;
    *bs->curr++ = 1;
    /* The next 1 byte is NALU header */
    *bs->curr++ = nal;

    bs->bits_pos += 8*5;
    bs->left_count = 16;
    bs->code_bits = 0;

    return 8*5;
}

//------------------------------------------------------------------------------
//  Function    : AvcWriteSPS
//  Description : This function write SPS to buffer.
//------------------------------------------------------------------------------
int AvcWriteSPS(bs_t* bs, sps_t* sps)
{
    int bit_pos = AvcCount(bs);

    BsWrite(bs, 8, (uint)sps->i_profile_idc);
    BsWriteU1(bs, sps->b_constraint_set0);
    BsWriteU1(bs, sps->b_constraint_set1);
    BsWriteU1(bs, sps->b_constraint_set2);
    BsWriteU1(bs, sps->b_constraint_set3);
    BsWrite(bs, 4, 0);

    BsWrite(bs, 8, (uint)sps->i_level_idc);
    BsWriteUE(bs, sps->i_sps_id);
    /* not support high profile
    if (sps->i_profile_idc > PROFILE_HIGH)
    {
        chroma_format_idc
        ....
    }
    */
    BsWriteUE(bs, sps->i_log2_max_frame_num - 4);
    BsWriteUE(bs, sps->i_poc_type);
    if (sps->i_poc_type == 0)
    {
        BsWriteUE(bs, sps->i_log2_max_poc_lsb - 4);
    }
    /* not support poc_type == 1
    else if (sps->i_poc_type == 1)
    {
      ....
    }
    */
    BsWriteUE(bs, sps->i_num_ref_frames);
    BsWriteU1(bs, sps->b_gaps_in_frame_num_value_allow);
    BsWriteUE(bs, sps->i_mb_w - 1);
    BsWriteUE(bs, sps->i_mb_h - 1);
    BsWriteU1(bs, 1/*frame_mbs_only_flag*/);
    /* only support frame_mbs */
    BsWriteU1(bs, sps->b_direct8x8_inference);
    BsWriteU1(bs, sps->b_crop);
    if (sps->b_crop)
    {
        BsWriteUE(bs, sps->crop.i_left);
        BsWriteUE(bs, sps->crop.i_right);
        BsWriteUE(bs, sps->crop.i_top);
        BsWriteUE(bs, sps->crop.i_bottom);
    }

    BsWriteU1(bs, sps->b_vui_param_pres);
    if (sps->b_vui_param_pres)
        VuiWriteParam(bs, &sps->vui);

    BsTrail(bs);

    return bs->bits_pos - bit_pos;
}

//------------------------------------------------------------------------------
//  Function    : VuiGetAspectRatioIdc
//  Description : This function Get Aspect Ratio Idc for VUI.
//------------------------------------------------------------------------------
int VuiGetAspectRatioIdc(int sar_w, int sar_h)
{
    int sar_idc = 0;

    if(sar_w == 0 || sar_h == 0)  /* unspecified */
        sar_idc = 0;
    else if(sar_w == sar_h)  /* square, 1:1 */
        sar_idc = 1;
    else if(sar_h == 11)
    {
        if(sar_w == 12)         /* 12:11 */
            sar_idc = 2;
        else if(sar_w == 10)    /* 10:11 */
            sar_idc = 3;
        else if(sar_w == 16)    /* 16:11 */
            sar_idc = 4;
        else if(sar_w == 24)    /* 24:11 */
            sar_idc = 6;
        else if(sar_w == 20)    /* 20:11 */
            sar_idc = 7;
        else if(sar_w == 32)    /* 32:11 */
            sar_idc = 8;
        else if(sar_w == 18)    /* 18:11 */
            sar_idc = 10;
        else if(sar_w == 15)    /* 15:11 */
            sar_idc = 11;
        else                    /* Extended_SAR */
            sar_idc = EXTENDED_SAR;
    }
    else if(sar_h == 33)
    {
        if(sar_w == 40)         /* 40:33 */
            sar_idc = 5;
        else if(sar_w == 80)    /* 80:33 */
            sar_idc = 9;
        else if(sar_w == 64)    /* 64:33 */
            sar_idc = 12;
        else                    /* Extended_SAR */
            sar_idc = EXTENDED_SAR;
    }
    else if(sar_w == 160 && sar_h == 99)    /* 160:99 */
        sar_idc = 13;
    else if(sar_w == 4 && sar_h == 3)   /* 4:3 */
        sar_idc = 14;
    else if(sar_w == 3 && sar_h == 2)   /* 3:2 */
        sar_idc = 15;
    else if(sar_w == 2 && sar_h == 1)   /* 2:1 */
        sar_idc = 16;
    else                                /* Extended_SAR */
        sar_idc = EXTENDED_SAR;

    return (sar_idc);
}


//------------------------------------------------------------------------------
//  Function    : VuiWriteParam
//  Description : This function write VUI to buffer.
//------------------------------------------------------------------------------
static int VuiWriteParam(bs_t* bs, vui_t* vui)
{
    int bit_pos = AvcCount(bs);

    //int vui->i_aspect_ratio_idc = VuiGetAspectRatioIdc(vui->i_sar_w, vui->i_sar_h);

    if(0 == vui->b_aspect_ratio_info_present_flag)
    {
        BsWriteU1(bs, 0/* aspect_ratio_info_present_flag */);
    }
    else
    {
        BsWriteU1(bs, 1/* aspect_ratio_info_present_flag */);
        BsWrite(bs, 8, vui->i_aspect_ratio_idc /* aspect_ratio_idc */);
        if(vui->i_aspect_ratio_idc == EXTENDED_SAR)
        {
            BsWrite(bs, 16, vui->i_sar_w /* sar_width */);
            BsWrite(bs, 16, vui->i_sar_h /* sar_height */);
        }
    }

    BsWriteU1(bs, vui->b_overscan_info_present_flag/* overscan_info_present_flag */);
    if (vui->b_overscan_info_present_flag)
    {
        BsWriteU1(bs, vui->b_overscan_appropriate_flag/* b_overscan_appropriate_flag */);
    }

    BsWriteU1(bs, vui->b_video_signal_pres);
    if (vui->b_video_signal_pres)
    {
        BsWrite(bs, 3, vui->i_video_format);
        BsWriteU1(bs, vui->b_video_full_range);
        BsWriteU1(bs, vui->b_colour_desc_pres);
        if (vui->b_colour_desc_pres)
        {
            BsWrite(bs, 8, vui->i_colour_primaries);
            BsWrite(bs, 8, vui->i_transf_character);
            BsWrite(bs, 8, vui->i_matrix_coeffs);
        }
    }
    BsWriteU1(bs, 0/* chroma_loc_info_present_flag */);
    if (vui->b_timing_info_pres)
    {
        BsWriteU1(bs, 1/* timing_info_present_flag */);
        BsWrite32(bs, vui->i_num_units_in_tick/* num_units_in_tick */);
        BsWrite32(bs, vui->i_time_scale/* time_scale */);
        BsWriteU1(bs, vui->b_fixed_frame_rate/* fixed_frame_rate_flag */);
    }
    else
    {
        BsWriteU1(bs, 0/* timing_info_present_flag */);
    }
    BsWriteU1(bs, 0/* nal_hrd_parameters_present_flag */);
    BsWriteU1(bs, 0/* vcl_hrd_parameters_present_flag */);
    BsWriteU1(bs, 0/* pic_struct_present_flag */);
    BsWriteU1(bs, 0/* bitstream_restriction_flag */);
    return bs->bits_pos - bit_pos;
}

//------------------------------------------------------------------------------
//  Function    : AvcWritePPS
//  Description : This function write PPS to buffer.
//------------------------------------------------------------------------------
int AvcWritePPS(bs_t* bs, pps_t* pps)
{
    int bit_pos = AvcCount(bs);

    BsWriteUE(bs, pps->i_pps_id);
    BsWriteUE(bs, pps->sps->i_sps_id);
    BsWriteU1(bs, pps->b_cabac_i);
    BsWriteU1(bs, 0/*b_bottom_field_pic_order*/);
    BsWriteUE(bs, 0/*i_num_slice_groups - 1*/);
    BsWriteUE(bs, pps->i_num_ref_idx_l0_default_active - 1);
    BsWriteUE(bs, pps->i_num_ref_idx_l1_default_active - 1);
    /* not support weighted pred */
    BsWriteU1(bs, 0/*b_weighted_pred*/);
    BsWrite(bs, 2, 0/*b_weighted_bipred*/);
    BsWriteSE(bs, pps->i_pic_init_qp - 26);
    BsWriteSE(bs, pps->i_pic_init_qs - 26);
    BsWriteSE(bs, pps->i_cqp_idx_offset);
    BsWriteU1(bs, pps->b_deblocking_filter_control);
    BsWriteU1(bs, pps->b_constrained_intra_pred);
    BsWriteU1(bs, pps->b_redundant_pic_cnt);
    /* not support transform8x8/q-matrix */

    BsTrail(bs);

    return bs->bits_pos - bit_pos;
}

//------------------------------------------------------------------------------
//  Function    : AvcWritePPS
//  Description : This function write PPS to buffer.
//------------------------------------------------------------------------------
int AvcWritePPS2nd(bs_t* bs, pps_t* pps)
{
    int bit_pos = AvcCount(bs);

    BsWriteUE(bs, 1);
    BsWriteUE(bs, pps->sps->i_sps_id);
    BsWriteU1(bs, pps->b_cabac_p);
    BsWriteU1(bs, 0/*b_bottom_field_pic_order*/);
    BsWriteUE(bs, 0/*i_num_slice_groups - 1*/);
    BsWriteUE(bs, pps->i_num_ref_idx_l0_default_active - 1);
    BsWriteUE(bs, pps->i_num_ref_idx_l1_default_active - 1);
    /* not support weighted pred */
    BsWriteU1(bs, 0/*b_weighted_pred*/);
    BsWrite(bs, 2, 0/*b_weighted_bipred*/);
    BsWriteSE(bs, pps->i_pic_init_qp - 26);
    BsWriteSE(bs, pps->i_pic_init_qs - 26);
    BsWriteSE(bs, pps->i_cqp_idx_offset);
    BsWriteU1(bs, pps->b_deblocking_filter_control);
    BsWriteU1(bs, pps->b_constrained_intra_pred);
    BsWriteU1(bs, pps->b_redundant_pic_cnt);
    /* not support transform8x8/q-matrix */

    BsTrail(bs);

    return bs->bits_pos - bit_pos;
}

//------------------------------------------------------------------------------
//  Function    : AvcWriteSliceHeader
//  Description : This function write slice header to buffer.
//------------------------------------------------------------------------------
int AvcWriteSliceHeader(bs_t* bs, slice_t* sh)
{
    BsWriteUE(bs, sh->i_first_mb);
    BsWriteUE(bs, sh->i_type);
    if ((sh->pps->b_cabac_i != sh->pps->b_cabac_p) && sh->i_type == SLICE_P)
        BsWriteUE(bs, 1);   // If I/P frame use different entropy type, P frame refer PPS_1
    else
        BsWriteUE(bs, sh->pps->i_pps_id);
    BsWrite(bs, sh->sps->i_log2_max_frame_num, (uint)sh->i_frm_num & ((1 << sh->sps->i_log2_max_frame_num) - 1));

    if (sh->b_idr_pic)
        BsWriteUE(bs, sh->i_idr_pid);
    if (sh->sps->i_poc_type == 0)
        BsWrite(bs, sh->sps->i_log2_max_poc_lsb, (uint)sh->i_poc & ((1 << sh->sps->i_log2_max_poc_lsb) - 1));
    if (sh->pps->b_redundant_pic_cnt)
        BsWriteUE(bs, sh->i_redundant_pic_cnt);
    if (sh->i_type == SLICE_B)
        BsWriteU1(bs, sh->b_direct_spatial_mv_pred);
    if (sh->i_type == SLICE_P || sh->i_type == SLICE_B)
    {
        BsWriteU1(bs, sh->b_num_ref_idx_override);
        if (sh->b_num_ref_idx_override)
        {
            BsWriteUE(bs, sh->i_num_ref_idx_l0_active - 1);
            if (sh->i_type == SLICE_B)
                BsWriteUE(bs, sh->i_num_ref_idx_l1_active - 1);
        }
    }
    /* not support ref_pic_list_reordering */
    if (sh->i_type != SLICE_I)
    {
        /* LTR is not enable */
        if (!sh->b_long_term_reference)
        {
            /* ref_pic_list_modification_flag_l0 */
            BsWriteU1(bs, 0);
        }
        else
        {
            int i;
            /* ref_pic_list_modification_flag_l0 */
            BsWriteU1(bs, sh->b_ref_pic_list_modification_flag_l0);
            /* fill ref pic list if LTR P-frame */
            if (sh->b_ref_pic_list_modification_flag_l0)
            {
                for (i = 0; i < MAX_MULTI_REF_FRAME_PLUS1; i++)
                {
                    /* modification_of_pic_nums_idc */
                    BsWriteUE(bs, sh->rpl0_t[i].modification_of_pic_nums_idc);
                    if (sh->rpl0_t[i].modification_of_pic_nums_idc == 0 ||
                        sh->rpl0_t[i].modification_of_pic_nums_idc == 1)
                    {
                        /* abs_diff_pic_num_minus1 */
                        BsWriteUE(bs, sh->rpl0_t[i].abs_diff_pic_num_minus1);
                    }
                    else if (sh->rpl0_t[i].modification_of_pic_nums_idc == 2)
                    {
                        /* long_term_pic_num */
                        BsWriteUE(bs, sh->rpl0_t[i].long_term_pic_num);
                    }
                    else if (sh->rpl0_t[i].modification_of_pic_nums_idc == 4 ||
                             sh->rpl0_t[i].modification_of_pic_nums_idc == 5)
                    {
                        /* abs_diff_view_idx_minus1 */
                        BsWriteUE(bs, sh->rpl0_t[i].abs_diff_view_idx_minus1);
                    }
                    else if (sh->rpl0_t[i].modification_of_pic_nums_idc == 3)
                    {
                        break;
                    }
                }
            }
        }
    }

    if (sh->i_type == SLICE_B)
        BsWriteU1(bs, 0);
    /* not support weighted pred */
    /* dec_ref_pic_marking() */
    if (sh->i_ref_idc)
    {
        /* I-frame */
        if (sh->b_idr_pic)
        {
            /* no_output_prior_pics_flag */
            BsWriteU1(bs, 0);
            /* long_term_reference_flag */
            if (!sh->b_long_term_reference)
                BsWriteU1(bs, 0);
            else
                BsWriteU1(bs, 1);
        }
        else
        {
            int i;
            /* adaptive_ref_pic_marking_mode_flag */
            if(!sh->b_adaptive_ref_pic_marking_mode)
                BsWriteU1(bs, 0);
            else
            {
                BsWriteU1(bs, 1);
                for (i = 0; i < MAX_MULTI_REF_FRAME_PLUS1; i++)
                {
                    /* memory_management_control_operation */
                    BsWriteUE(bs, sh->mmc_t[i].memory_management_control);
                    if(sh->mmc_t[i].memory_management_control == 1 ||
                       sh->mmc_t[i].memory_management_control == 3)
                    {
                        /* difference_of_pic_nums_minus1 */
                        BsWriteUE(bs, sh->mmc_t[i].difference_of_pic_nums_minus1);
                    }
                    if(sh->mmc_t[i].memory_management_control == 2)
                    {
                        /* ong_term_pic_num */
                        BsWriteUE(bs, sh->mmc_t[i].long_term_pic_num);
                    }
                    if(sh->mmc_t[i].memory_management_control == 3 ||
                       sh->mmc_t[i].memory_management_control == 6)
                    {
                        /* long_term_frame_idx */
                        BsWriteUE(bs, sh->mmc_t[i].long_term_frame_idx);
                    }
                    if(sh->mmc_t[i].memory_management_control == 4)
                    {
                        /* max_long_term_frame_idx_plus1 */
                        BsWriteUE(bs, sh->mmc_t[i].max_long_term_frame_idx_plus1);
                    }
                }
            }
        }
    }
    if (sh->pps->b_cabac_p && sh->i_type != SLICE_I)
        BsWriteUE(bs, sh->i_cabac_init_idc);
    BsWriteSE(bs, sh->i_qp - sh->pps->i_pic_init_qp);
    if (sh->pps->b_deblocking_filter_control)
    {
        BsWriteUE(bs, sh->i_disable_deblocking_filter_idc);
        if (sh->i_disable_deblocking_filter_idc != 1)
        {
            BsWriteSE(bs, sh->i_alpha_c0_offset_div2);
            BsWriteSE(bs, sh->i_beta_offset_div2);
        }
    }
    if (sh->i_type == SLICE_I)
    {
        if (sh->pps->b_cabac_i)
            BsAlign(bs, 1);
    }
    else
    {
        if (sh->pps->b_cabac_p)
            BsAlign(bs, 1);
    }

    return bs->bits_pos;
}
