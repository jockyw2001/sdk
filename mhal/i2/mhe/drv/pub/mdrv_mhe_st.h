////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2011 MStar Semiconductor, Inc.
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

//! @file mdrv_mmhe_st.h
//! @author MStar Semiconductor Inc.
//! @brief MHE Driver IOCTL User's Interface.
//! \ingroup mmhe_group MHE driver
//! @{
#ifndef _MDRV_MMHE_ST_H_
#define _MDRV_MMHE_ST_H_

#define MMHEIF_MAJ              1   //!< major version: Major number of driver-I/F version.
#define MMHEIF_MIN              2   //!< minor version: Minor number of driver-I/F version.
#define MMHEIF_EXT              0   //!< extended code: Extended number of version. It should increase when "mdrv_mmhe_io.h/mdrv_mmhe_st.h" changed.

//! User Interface version number.
#define MMHEIF_VERSION_ID       ((MMHEIF_MAJ<<22)|(MMHEIF_MIN<<12)|(MMHEIF_EXT))
//! Acquire version number.
#define MMHEIF_GET_VER(v)       (((v)>>12))
//! Acquire major version number.
#define MMHEIF_GET_MJR(v)       (((v)>>22)&0x3FF)
//! Acquire minor version number.
#define MMHEIF_GET_MNR(v)       (((v)>>12)&0x3FF)
//! Acquire extended number.
#define MMHEIF_GET_EXT(v)       (((v)>> 0)&0xFFF)

//! mmhe_pixfmt indicates pixels formats
enum mmhe_pixfmt
{
    MMHE_PIXFMT_NV12 = 0,   //!< pixel format NV12.
    MMHE_PIXFMT_NV21,       //!< pixel format NV21.
    MMHE_PIXFMT_YUYV,       //!< pixel format YUYV.
    MMHE_PIXFMT_YVYU,       //!< pixel format YVYU.
};

//! mmhe_subpel indicates interpolation granularity
enum mmhe_subpel
{
    MMHE_SUBPEL_INT = 0,    //!< subpel integral.
    MMHE_SUBPEL_HALF,       //!< subpel half.
    MMHE_SUBPEL_QUATER,     //!< subpel quater.
};

#define MMHE_MVBLK_4x4          (1<<0)  //!< bit-field of  4x4  block
#define MMHE_MVBLK_8x4          (1<<1)  //!< bit-field of  8x4  block
#define MMHE_MVBLK_4x8          (1<<2)  //!< bit-field of  4x8  block
#define MMHE_MVBLK_8x8          (1<<3)  //!< bit-field of  8x8  block
#define MMHE_MVBLK_16x8         (1<<4)  //!< bit-field of 16x8  block
#define MMHE_MVBLK_8x16         (1<<5)  //!< bit-field of  8x16 block
#define MMHE_MVBLK_16x16        (1<<6)  //!< bit-field of 16x16 block
#define MMHE_MVBLK_SKIP         (1<<7)  //!< bit-field of skip

#define MVHE_HEVC_PROFILE_MAIN       1  //!< hevc main   profile
#define MVHE_HEVC_PROFILE_MAIN10     2  //!< hevc main10 profile
#define MVHE_HEVC_LEVEL_1           30  //!< hevc level 1.0
#define MVHE_HEVC_LEVEL_2           60  //!< hevc level 2.0
#define MVHE_HEVC_LEVEL_2_1         63  //!< hevc level 2.1
#define MVHE_HEVC_LEVEL_3           90  //!< hevc level 3.0
#define MVHE_HEVC_LEVEL_3_1         93  //!< hevc level 3.1
#define MVHE_HEVC_LEVEL_4          120  //!< hevc level 4.0
#define MVHE_HEVC_LEVEL_4_1        124  //!< hevc level 4.1
#define MVHE_HEVC_LEVEL_5          150  //!< hevc level 5.0
#define MVHE_HEVC_LEVEL_5_1        153  //!< hevc level 5.1
#define MVHE_HEVC_LEVEL_5_2        156  //!< hevc level 5.2
#define MVHE_HEVC_LEVEL_6          180  //!< hevc level 6.0
#define MVHE_HEVC_LEVEL_6_1        183  //!< hevc level 6.1
#define MVHE_HEVC_LEVEL_6_2        186  //!< hevc level 6.2
#define MVHE_FLAGS_CMPRY        (1<< 0)
#define MVHE_FLAGS_COMPR        (MVHE_FLAGS_CMPRY)

//! mmhe_parm is used to apply/query configs out of streaming period.
typedef union mmhe_parm
{
    //! indicating parameter type.
    enum mmhe_parm_e
    {
        MMHE_PARM_IDX = 0,  //!< parameters of streamid: query stream-id.
        MMHE_PARM_RES,      //!< parameters of resource: including image's resolution and format
        MMHE_PARM_FPS,      //!< parameters of fps: fraction of framerate.
        MMHE_PARM_GOP,      //!< parameters of gop: ip frame period.
        MMHE_PARM_BPS,      //!< parameters of bps: bit per second.
        MMHE_PARM_MOT,      //!< parameters of motion: interpolation granularity, mv-partition.
        MMHE_PARM_HEVC,      //!< parameters of hevc: codec settings.
        MMHE_PARM_AVC,      //!< parameters of avc: codec settings.
        MMHE_PARM_VUI,      //!< parameters of vui: vui params.
        MMHE_PARM_LTR,      //!< parameters of ltr: long term reference.
    } type;             //!< indicating which kind of mmhe_parm is.

    //! set res parameter out of streaming.
    struct mmhe_parm_idx
    {
        enum mmhe_parm_e    i_type;         //!< i_type MUST be MMHE_PARM_IDX.
        int                 i_stream;       //!< stream-id.
    } idx;

    //! set res parameter out of streaming.
    struct mmhe_parm_res
    {
        enum mmhe_parm_e    i_type;         //!< i_type MUST be MMHE_PARM_RES.
        int                 i_pict_w;       //!< picture width.
        int                 i_pict_h;       //!< picture height.
        int                 i_crop_w;
        int                 i_crop_h;
        int                 i_crop_offset_x;
        int                 i_crop_offset_y;
        enum mmhe_pixfmt    i_pixfmt;       //!< pixel format.
        int                 i_outlen;       //!< output length: '<0' mmap-mode, '>=0' user-mode.
        int                 i_flags;        //!< flags.
        char                i_ClkEn;
        char                i_ClkSor;
    } res;              //!< used to set resource parameters.

    //! set fps parameter out of streaming.
    struct mmhe_parm_fps
    {
        enum mmhe_parm_e    i_type;         //!< i_type MUST be MMHE_PARM_FPS.
        int                 i_num;          //!< numerator of fps.
        int                 i_den;          //!< denominator of fps.
    } fps;              //!< used to set fraction of frame rate.

    //! set mot parameter out of streaming.
    struct mmhe_parm_mot
    {
        enum mmhe_parm_e    i_type;         //!< i_type MUST be MMHE_PARM_MOT.
        int                 i_dmv_x;        //!< x-dir dmv.
        int                 i_dmv_y;        //!< y-dir dmv.
        enum mmhe_subpel    i_subpel;       //!< interpolation granularity.
        unsigned int        i_mvblks[2];    //!< mv-partitions.
    } mot;              //!< used to set motion configuratiion.

    //! set gop parameter out of streaming.
    struct mmhe_parm_gop
    {
        enum mmhe_parm_e    i_type;         //!< i_type MUST be MMHE_PARM_GOP.
        int                 i_pframes;      //!< p-frames count per i-frame
        int                 i_bframes;      //!< b-frames count per i/p-frame
        unsigned char       b_passiveI;     //!< Passive encode I frame.
    } gop;              //!< used to set gop structure.

    //! set bps parameter out of streaming.
    struct mmhe_parm_bps
    {
        enum mmhe_parm_e    i_type;         //!< i_type MUST be MMHE_PARM_BPS.
        int                 i_method;       //!< rate-control method.
        int                 i_ref_qp;       //!< ref. QP.
        int                 i_delta_qp;     //!< ref. QP Iframe delta.
        int                 i_bps;          //!< bitrate.
    } bps;              //!< used to set bit rate controller.

    //! set hevc parameters out of streaming.
    struct mmhe_parm_hevc
    {
        enum mmhe_parm_e    i_type;                     //!< i_type MUST be MVHE_PARM_HEVC.
        unsigned short      i_profile;                  //!< profile.
        unsigned short      i_level;                    //!< level.
        unsigned char       i_log2_max_cb_size;         //!< max ctb size.
        unsigned char       i_log2_min_cb_size;         //!< min ctb size.
        unsigned char       i_log2_max_tr_size;         //!< max trb size.
        unsigned char       i_log2_min_tr_size;         //!< min trb size.
        unsigned char       i_tr_depth_intra;           //!< tr depth intra.
        unsigned char       i_tr_depth_inter;           //!< tr depth inter.
        unsigned char       b_scaling_list_enable;      //!< scaling list enable.
        unsigned char       b_ctu_qp_delta_enable;      //!< ctu dqp enable.
        unsigned char       b_sao_enable;               //!< sao enable.
        signed char         i_cb_qp_offset;             //!< cb qp offset: -12 to 12 (inclusive)
        signed char         i_cr_qp_offset;             //!< cr qp offset: -12 to 12 (inclusive)
        unsigned char       b_strong_intra_smoothing;   //!< strong intra smoothing.
        unsigned char       b_constrained_intra_pred;   //!< intra prediction constrained.
        unsigned char       b_deblocking_override_enable;   //!< deblocking override enable.
        unsigned char       b_deblocking_disable;           //!< deblocking disable.
        unsigned char       b_deblocking_cross_slice_enable;//!< deblocking cross slice boundary enable.
        signed char       i_tc_offset_div2;           //!< tc_offset_div2: -6 to 6 (inclusive)
        signed char       i_beta_offset_div2;         //!< beta_offset_div2: -6 to 6 (inclusive)
    } hevc;             //!< used to set codec configuration.

    /*
        //! set vui parameter out of streaming.
        struct mmhe_parm_vui {
            enum mmhe_parm_e    i_type;         //!< i_type MUST be MVHE_PARM_VUI.
            int                 b_video_full_range;
        } vui;              //!< used to set codec configuration.
    */

    //! set avc parameter out of streaming.
    struct mmhe_parm_avc
    {
        enum mmhe_parm_e    i_type;                     //!< i_type MUST be MMHE_PARM_AVC.
        unsigned short      i_profile;                  //!< profile.
        unsigned short      i_level;                    //!< level.
        unsigned char       i_num_ref_frames;           //!< ref.frames count.
        unsigned char       i_poc_type;                 //!< poc_type: support 0,2.
        unsigned char       b_cabac;                    //!< entropy: cabac/cavlc.
        unsigned char       b_constrained_intra_pred;   //!< contrained intra pred.
        unsigned char       b_deblock_filter_control;   //!< deblock filter control.
        unsigned char       i_disable_deblocking_idc;   //!< disable deblocking idc.
        signed char       i_alpha_c0_offset;          //!< offset alpha div2.
        signed char       i_beta_offset;              //!< offset beta div2.
    } avc;              //!< used to set codec configuration.

    //! set avc parameter out of streaming.
    struct mmhe_parm_vui
    {
        enum mmhe_parm_e    i_type;         //!< i_type MUST be MMHE_PARM_VUI.
        int                 b_aspect_ratio_info_present_flag;
        int                 i_aspect_ratio_idc;
        int                 i_sar_w;
        int                 i_sar_h;
        int                 b_overscan_info_present_flag;
        int                 b_overscan_appropriate_flag;
        int                 b_video_full_range;
        int                 b_video_signal_pres;
        int                 i_video_format;
        int                 b_colour_desc_pres;
        int                 i_colour_primaries;
        int                 i_transf_character;
        int                 i_matrix_coeffs;
        int                 b_timing_info_pres;
        int                 i_num_units_in_tick;
        int                 i_time_scale;
        int                 i_num_ticks_poc_diff_one_minus1;
    } vui;              //!< used to set codec configuration.

    //! set ltr parameter out of streaming.
    struct mmhe_parm_ltr
    {
        enum mmhe_parm_e    i_type;                     //!< i_type MUST be MVHE_PARM_LTR.
        int                 b_long_term_reference;      //!< toggle ltr mode
        int                 b_enable_pred;              //!< ltr mode, means LTR P-frame can be ref.; 0: P ref. I, 1: P ref. P
        int                 i_ltr_period;               //!< ltr period
    } ltr;              //!< used to set ltr configuration.

    unsigned char           byte[64];   //!< dummy bytes

} mmhe_parm;

//! mmhe_ctrl is used to apply/query control configs during streaming period.
typedef union mmhe_ctrl
{
    //! indicating control type.
    enum  mmhe_ctrl_e
    {
        MMHE_CTRL_SEQ = 0,  //!< control of sequence: includes resolution, pixel format and frame-rate.
        MMHE_CTRL_HEVC,     //!< control of hevc codec settings.
        MMHE_CTRL_ROI,      //!< control of roi setting changing.
        MMHE_CTRL_BGFPS,     //!< control of roi background frame rate changing
        MMHE_CTRL_QPMAP,    //!< control of qpmap setting changing.
        MMHE_CTRL_SPL,      //!< control of slice spliting.
        MMHE_CTRL_DBK,      //!< control of deblocking.
        MMHE_CTRL_BAC,      //!< control of cabac_init.
        MMHE_CTRL_LTR,      //!< control of long term reference.
        MMHE_CTRL_FME,      //!< control of frame cfg: qp offset, i/p bits threshold
        MMHE_CTRL_RST,      //!< control of restore ops pointer
    } type;                 //!< indicating which kind of mmhe_ctrl is.

    //! set seq parameter during streaming.
    struct mmhe_ctrl_seq
    {
        enum mmhe_ctrl_e    i_type;             //!< i_type MUST be MMHE_CTRL_SEQ.
        enum mmhe_pixfmt    i_pixfmt;           //!< pixel-format
        short               i_pixelw;           //!< pixels in width
        short               i_pixelh;           //!< pixels in height
        short               n_fps;              //!< numerator of frame-rate
        short               d_fps;              //!< denominator of frame-rate
    } seq;              //!< used to start new sequence.

    //! set avc parameter during streaming.
    struct mmhe_ctrl_avc
    {
        enum mmhe_ctrl_e    i_type;                     //!< i_type MUST be MMHE_CTRL_AVC.
        unsigned short      i_profile;                  //!< profile.
        unsigned short      i_level;                    //!< level.
        unsigned char       i_num_ref_frames;           //!< ref.frames count.
        unsigned char       i_poc_type;                 //!< poc_type: support 0,2.
        unsigned char       b_cabac;                    //!< entropy: cabac/cavlc.
        unsigned char       b_constrained_intra_pred;   //!< contrained intra pred.
        unsigned char       b_deblock_filter_control;   //!< deblock filter control.
        unsigned char       i_disable_deblocking_idc;   //!< disable deblocking idc.
        signed char         i_alpha_c0_offset;          //!< offset alpha div2.
        signed char         i_beta_offset;              //!< offset beta div2.
    } avc;              //!< used to set avc codec setting.

    //! set roi parameter during streaming.
    struct mmhe_ctrl_roi
    {
        enum mmhe_ctrl_e    i_type;             //!< i_type MUST be MMHE_CTRL_ROI.
        short               i_index;            //!< roi index.
        short               i_absqp;            //!< roi abs-qp: 1: roiqp is absqp, 0: roiqp is offset qp
        short               i_roiqp;            //!< roi roi-qp: 0-disable roi, negative value.
        unsigned short      i_cbx;              //!< roi region posotion-X. (in CTB unit)
        unsigned short      i_cby;              //!< roi region posotion-Y. (in CTB unit)
        unsigned short      i_cbw;              //!< roi region rectangle-W. (in CTB unit)
        unsigned short      i_cbh;              //!< roi region rectangle-H. (in CTB unit)
    } roi;              //!< used to set roi region and dqp.

    //! set roi background frame rate parameter during streaming.
    struct mmhe_ctrl_bgfps
    {
        enum mmhe_ctrl_e    i_type;             //!< i_type MUST be MMHE_CTRL_BGFPS.
        int                 i_bgsrcfps;         //!< roi background source frame rate.
        int                 i_bgdstfps;         //!< roi background destination frame rate.
    } bgfps;              //!< used to set roi background frame rate.

    //! set qpmap parameter during streaming.
    struct mmhe_ctrl_qpmap
    {
        enum mmhe_ctrl_e    i_type;             //!< i_type MUST be MMHE_CTRL_QPMAP.
        int                 i_enb;              //!< qpmap enable.
        signed char         i_entry[14];        //!< qpmap entry: qp offset (-51 ~ 51).
        unsigned char*      p_mapkptr;          //!< qpmap virtual address.
    } qpmap;

    //! set spl parameter during streaming.
    struct mmhe_ctrl_spl
    {
        enum mmhe_ctrl_e    i_type;             //!< i_type MUST be MMHE_CTRL_SPL.
        int                 i_rows;             //!< slice split by mb-rows.
        int                 i_bits;             //!< slice split by bitcnt.
    } spl;              //!< used to set slice splitting.

    //! set dbk control during streaming.
    struct mmhe_ctrl_dbk
    {
        enum mmhe_ctrl_e    i_type;             //!< i_type MUST be MVHE_CTRL_DBK.
        unsigned char       b_override;         //!< override deblocking setting.
        unsigned char       b_disable;          //!< deblocking disable.
        signed char       i_tc_offset_div2;   //!< tc_offset_div2: -6 to 6 (inclusive)
        signed char       i_beta_offset_div2; //!< beta_offset_div2: -6 to 6 (inclusive)
    } dbk;              //!< used to set deblocking splitting.

    //! set bac control during streaming.
    struct mmhe_ctrl_bac
    {
        enum mmhe_ctrl_e    i_type;     //!< i_type MUST be MVHE_CTRL_BAC.
        int                 b_init;     //!< cabac_init_flag: 0,1
    } bac;              //!< used to set cabac_init.

    //! set ltr parameter during streaming.
    struct mmhe_ctrl_ltr
    {
        enum mmhe_ctrl_e    i_type;             //!< i_type MUST be MMHE_CTRL_LTR.
        int                 b_enable_pred;      //!< ltr mode, means LTR P-frame can be ref.; 0: P ref. I, 1: P ref. P
        int                 i_ltr_period;       //!< ltr period
    } ltr;              //!< used to set long term reference.

    // ! set frame configuration during streaming
    struct mmhe_ctrl_fme {
        enum mmhe_ctrl_e    i_type;             //!< i_type MUST be MMFE_CTRL_FME.
        signed char i_qp_offset;
        unsigned int i_iframe_bits_threshold;
        unsigned int i_pframe_bits_threshold;
    } fme;

    // ! set restore parameter during streaming
    struct mmhe_ctrl_rst {
        enum mmhe_ctrl_e i_type;               //!< i_type MUST be MMFE_CTRL_RST.
    } rst;

    unsigned char           byte[64];   //!< dummy bytes
} mmhe_ctrl;

#define MMHE_FLAGS_IDR        (1<< 0)   //!< request IDR.
#define MMHE_FLAGS_DISPOSABLE (1<< 1)   //!< request unref-pic.
#define MMHE_FLAGS_NIGHT_MODE (1<< 2)   //!< night mode.
#define MMHE_FLAGS_SOP        (1<<30)   //!< start of picture.
#define MMHE_FLAGS_EOP        (1<<31)   //!< end of picture.

#define MMHE_MEMORY_USER      (0)       //!< user mode. (pass pointer)
#define MMHE_MEMORY_MMAP      (1)       //!< mmap mode. (pass physic address)

//!  mmhe_buff is used to exchange video/obits buffer between user and driver during streaming period.
typedef struct mmhe_buff
{
    int             i_index;    //!< index of buffer: '-1' invalid, '>=0' valid.
    int             i_flags;    //!< flags for request/reports.
    short           i_memory;   //!< memory mode of user/mmap.
    short           i_width;    //!< pixels in width. (if buffer is image)
    short           i_height;   //!< pixels in height. (if buffer is image)
    short           i_stride;   //!< pixels in stride. (if buffer is image) '<width': stride=width.
    long long       i_timecode; //!< timestamp of buffer.
    int             reserved;   //!< reserved.
    int             i_motion;   //!< measurement of motion.
    int             i_others;   //!< measurement of others.
    int             i_planes;   //!< buffer planes count.
    struct {
        struct {
            unsigned long long  phys;   //!< physical address.
            void*               uptr;   //!< virtual pointers.
            int                 (*pFlushCacheCb)(void *va, unsigned int len);
        } mem;          //!< memory address for virtual pointer or physical address.
        int         i_bias;
        int         i_size;     //!< memory occupied size.
        int         i_used;     //!< memory used size.
    } planes[2];    //!< planar structure.
} mmhe_buff;

#endif //_MDRV_MMHE_ST_H_
//! @}
