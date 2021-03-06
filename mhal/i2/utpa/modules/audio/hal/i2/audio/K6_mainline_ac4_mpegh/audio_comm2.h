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
#ifndef _AUDIO_COMM2_H_
#define _AUDIO_COMM2_H_

#include "audio_mbox2.h"
#include "ddr_config.h"
#include "dsp_version.h"

#ifdef _COMPILE_DSP_
    #include "Sys_def.h"
#endif

/************************************************
*  請保持 Utopia 和 DSP 裡的這個檔案一致
*
*  1. 註解不要用 !
*  2. 宣告不要用 .const xxxx = ????;
************************************************/


/*********************************************************
*   Version Control
*********************************************************/
#define  dde_version_num                 0xD20053
#define  ms10_dde_version_num            0xD800CA
#define  btscEnc_version_num             0xEF0127
#define  fmTx_version_num                0xED010D

#define  AUDIO_DSP2_VERSION    (system_version_num + dde_version_num + ms10_dde_version_num + btscEnc_version_num + fmTx_version_num)


/*********************************************************
*   system define
*********************************************************/
    #define DSP2_DDP_HDMI_BYPASS_EN                 0

    /* Software Feature List */
    #define SW_DMA_RDR_EN                           0               // SW_DMA_READER1, output by "decoder 5 = 0x8C", disable when HW_DMA_READER2_SUPPORT
    #define PCM_CAPTURE1_EN                         1               // PCM_CAPTURE1
    #define PCM_CAPTURE2_EN                         1               // PCM_CAPTURE2
    #define PCM_CAPTURE3_EN                         0               // PCM_CAPTURE3
    #define HW_DMA_WRITER1_EN                       0

    #define R2_DEC_ISR1_EN                          0               // replace R2_DMA_READER1 (R2_DECODER1_OUTPUT_BY_DSP)
    #define R2_DEC_ISR2_EN                          0               // replace R2_DMA_READER2 (R2_DECODER2_OUTPUT_BY_DSP)

    #define MULTI_CH_INPUT_DELAY_EN                 1               // multi-channel input delay func.
    #define CH5_INPUT_DELAY_EN                      1               // ch5 input delay func.
    #define CH6_INPUT_DELAY_EN                      0               // ch6 input delay func.
    #define HEADPHONE_DELAY_EN                      0               // Richard FMTX need this delay buffer
    #define HDMI_IND_DELAY                          1               // hdmi tx delay func.

    #define AVC_EN                                  1               // Auto Volume Control
    #define DRC_EN                                  1               // Dynamic Rnage Control
    #define GEQ_EN                                  1               // Graphic-EQ
    #define PEQ_EN                                  0               // Parametric-EQ
    #define HPF_EN                                  1               // High Pass filter
    #define NR_EN                                   1               // Noise Reduction
    #define BASS_TREBLE_EN                          1               // Basss Treble
    #define SURR_EN                                 1               // Surround
    #define BASSBOOST_EN                            0               // Super Bass
    #define DC_REMOVAL_EN                           1               // DC removal
    #define EASE_EN                                 1               // Netflix Ease function with fadding
    #define SWEEP_TONE_EN                           1               // Sweep sine func

    /* ASND System Channels */
        #define SE_PROCESS_FETCH_CHANNELS           14              // SE_Buffer input channels
        #define SE_PROCESS_STORE_CHANNELS           8               // SE_Buffer output channels
        #define SPDIF_DELAY_STORE_CHANNELS          2               // SPDIF Buffer channels
        #define HDMI_DELAY_STORE_CHANNELS           8               // HDMI Buffer channels, 8:_deciFs4x_
        #define DELAY_FUNCTION_STORE_CHANNELS       2               // Delay function Buffer channels
        #define MULTI_CH_INPUT_DELAY_STORE_CHANNELS 6               // multi-channel input delay function Buffer channels

    /* DSP Audio Delay Setting */
        #define AUDIO_DELAY_FS                      48              // fs = 48kHz
        #define SPDIF_DELAY_FS                      48
        #define HDMI_DELAY_FS                       48
        #define DMA24BIT_BYTES_IN_WORDS             3
        #define DMA16BIT_BYTES_IN_WORDS             2

        #define AUDIO_DELAY_LOWER_BOUND             0x20            // min main audio delay , 0x20 = 32 ms
        #define SPDIF_DELAY_LOWER_BOUND             0x05            // min spdif audio delay, 0x05 =  5 ms
        #define HDMI_DELAY_LOWER_BOUND              0x05            // min hdmi audio delay , 0x05 =  5 ms
        #define KTV_DELAY_LOWER_BOUND               0x14            // min ktv audio delay  , 0x14 = 20 ms
        #define AUDIO_DELAY_UPPER_BOUND             ((SE_MAIN_IN_DRAM_SIZE/SE_PROCESS_FETCH_CHANNELS + SE_MAIN_OUT_DRAM_SIZE/SE_PROCESS_STORE_CHANNELS)/DMA24BIT_BYTES_IN_WORDS/AUDIO_DELAY_FS) //unit : ms
        #define SPDIF_DELAY_UPPER_BOUND             ((SPDIF_DLY_DRAM_SIZE/SPDIF_DELAY_STORE_CHANNELS)/DMA16BIT_BYTES_IN_WORDS/SPDIF_DELAY_FS) //unit : ms
        #define HDMI_DELAY_UPPER_BOUND              ((SE_HDMI_DLY_DRAM_SIZE/2)/DMA16BIT_BYTES_IN_WORDS/HDMI_DELAY_FS) //unit : ms

        #define HEAD_PHONE_DLY_UPPER_BOUND          ((HEAD_PHONE_DLY_DRAM_SIZE/DELAY_FUNCTION_STORE_CHANNELS)/DMA24BIT_BYTES_IN_WORDS/AUDIO_DELAY_FS) //unit : ms
        #define CH5_INPUT_DLY_UPPER_BOUND           ((CH5_INPUT_DLY_DRAM_SIZE/DELAY_FUNCTION_STORE_CHANNELS)/DMA24BIT_BYTES_IN_WORDS/AUDIO_DELAY_FS) //unit : ms
        #define CH6_INPUT_DLY_UPPER_BOUND           ((CH6_INPUT_DLY_DRAM_SIZE/DELAY_FUNCTION_STORE_CHANNELS)/DMA24BIT_BYTES_IN_WORDS/AUDIO_DELAY_FS) //unit : ms
        #define MULTI_CH_INPUT_DLY_UPPER_BOUND      ((MULTI_CH_INPUT_DLY_DRAM_SIZE/MULTI_CH_INPUT_DELAY_STORE_CHANNELS)/DMA24BIT_BYTES_IN_WORDS/AUDIO_DELAY_FS) //unit : ms

    /* Audio Ease */
        #define AUDIO_EASE_TYPE_LINEAR              0
        #define AUDIO_EASE_TYPE_INCUBIC             1
        #define AUDIO_EASE_TYPE_OUTCUBIC            2
        #define AUDIO_EASE_TYPE_INOUTCUBIC          3

        #define AUDIO_EASE_CH_NONE                  0
        #define AUDIO_EASE_CH_A                     1
        #define AUDIO_EASE_CH_B                     2

    /* MixerMode_Ctrl */
        #define AUDIO_MIXER_GROUP_CH5               0   // 1: on; 0: off
        #define AUDIO_MIXER_GROUP_CH6               1
        #define AUDIO_MIXER_GROUP_CH7               2
        #define AUDIO_MIXER_GROUP_CH8               3

    /* Sine Tone */
        #define SINE_GEN_PARAM_FREQ_48k             137     // (3.1415926*2/8) * 8388608 / 48000
        #define SINE_GEN_PARAM_FREQ_32k             205     // (3.1415926*2/8) * 8388608 / 32000
        #define SINE_GEN_PARAM_STEPS                1024    // 1024 steps

/************************************************
* DSP sram address mapping
************************************************/
    /* DSP SRAM Segment */
        /* CM */
        #define DSP2_CM_MAIN_ADDR                   0x0
        #define DSP2_CM_MAIN_SIZE                   0x2000
        #define DSP2_CM_CODE1_ADDR                  0x2000
        #define DSP2_CM_CODE1_SIZE                  0x0100
        #define DSP2_CM_CODE2_ADDR                  0x2100
        #define DSP2_CM_CODE2_SIZE                  0x0000

        /* PM */
        #define DSP2_PM_MAIN_ADDR                   0x2100
        #define DSP2_PM_MAIN_SIZE                   0x02FF
        #define DSP2_PM_SEG1_ADDR                   0x23FF
        #define DSP2_PM_SEG1_SIZE                   0x2401
        #define DSP2_PM_SEG2_ADDR                   0x4800
        #define DSP2_PM_SEG2_SIZE                   0x0000

        /* Prefetch */
        #define DSP2_PM_PREFETCH_DSPADDR            0x10000                // check "arch.sys"
        #define DSP2_PM_PREFETCH_DDRADDR            DSP2_PM_PREFETCH_DSPADDR*3/BYTES_IN_MIU_LINE
        #define DSP2_PM_PREFETCH2_DSPADDR           0x12000                // check "arch.sys"
        #define DSP2_PM_PREFETCH2_DDRADDR           DSP2_PM_PREFETCH2_DSPADDR*3/BYTES_IN_MIU_LINE

        /* DM */
        #define DSP2_DM_MAIN_ADDR                   0x2F00
        #define DSP2_DM_MAIN_SIZE                   0x1100
        #define DSP2_DM_SEG1A_ADDR                  0x0
        #define DSP2_DM_SEG1A_SIZE                  0x2F00
        #define DSP2_DM_SEG1B_ADDR                  0x2F00
        #define DSP2_DM_SEG1B_SIZE                  0x0
        #define DSP2_DM_SEG2_ADDR                   0x2F00
        #define DSP2_DM_SEG2_SIZE                   0x0

        /* XBox */
        #define DSP2_XBOX_MAIN_ADDR                 0xB800
        #define DSP2_XBOX_MAIN_SIZE                 0x200

            #define ENC_INIT_ADDR           DSP2_CM_CODE1_ADDR
            #define ENC_CM_SEG              _cm_code1
            #define ENC_PM_SEG              _pm_ovly1
            #define ENC_CACHE_SEG           _ext_code05
            #define ENC_FETCH_SEG           _ext_fetch1
            #define ENC_DM_SEG              _dm_ovly1a
            #define ENC_PM_SEG_ADDR         DSP2_PM_SEG1_ADDR
            #define ENC_DM_SEG_ADDR         DSP2_DM_SEG1A_ADDR

    /* SND DSP PM vars */
        /* common */
            #define DSP2PmAddr_mainVer              (DSP2_PM_MAIN_ADDR)         //0x1900
            #define DSP2PmAddr_alg1Ver              (DSP2_PM_SEG1_ADDR)         //0x1BFF      // decoder 2 version
            #define DSP2PmAddr_alg2Ver              (DSP2_PM_MAIN_ADDR-1)

            #define DSP2PmAddr_mainAudioDelay       (DSP2PmAddr_mainVer +   0x01)
            #define DSP2PmAddr_spdifDelay           (DSP2PmAddr_mainVer +   0x02)
            #define DSP2PmAddr_hpDelay              (DSP2PmAddr_mainVer +   0x03)
            #define DSP2PmAddr_hdmiNonPcmSts        (DSP2PmAddr_mainVer +   0x04)
            #define DSP2PmAddr_btFrameSize          (DSP2PmAddr_mainVer +   0x05)
            #define DSP2PmAddr_ipSecurity           (DSP2PmAddr_mainVer +   0x06)
            #define DSP2PmAddr_hdmiDelay            (DSP2PmAddr_mainVer +   0x07)
            #define DSP2PmAddr_ch5InputDelay        (DSP2PmAddr_mainVer +   0x08)
            #define DSP2PmAddr_ch6InputDelay        (DSP2PmAddr_mainVer +   0x09)
            #define DSP2PmAddr_multiChInputDelay    (DSP2PmAddr_mainVer +   0x0A)
            #define DSP2PmAddr_ipSecurity2          (DSP2PmAddr_mainVer +   0x0B)

            #define DSP2PmAddr_peq48KCoeffAddr      (DSP2PmAddr_mainVer +   0x10)
            #define DSP2PmAddr_peq32KCoeffAddr      (DSP2PmAddr_peq48KCoeffAddr)
            #define DSP2PmAddr_peqscale48KAddr      (DSP2PmAddr_peq48KCoeffAddr     + 40 )
            #define DSP2PmAddr_peqbandEnAddr        (DSP2PmAddr_peqscale48KAddr     + 8  )
            #define DSP2PmAddr_peqbandDoubleAddr    (DSP2PmAddr_peqbandEnAddr       + 1  )
            #define DSP2PmAddr_hpf48KCoeffAddr      (DSP2PmAddr_peqbandDoubleAddr   + 1  )
            #define DSP2PmAddr_toneSelectAddr       (DSP2PmAddr_hpf48KCoeffAddr     + 5  )
            #define DSP2PmAddr_bass48KCoeffAddr     (DSP2PmAddr_toneSelectAddr      + 1  )
            #define DSP2PmAddr_bassscale48KAddr     (DSP2PmAddr_bass48KCoeffAddr    + 5  )
            #define DSP2PmAddr_treble48KCoeffAddr   (DSP2PmAddr_bassscale48KAddr    + 1  )
            #define DSP2PmAddr_treblescale48KAddr   (DSP2PmAddr_treble48KCoeffAddr  + 5  )
            #define DSP2PmAddr_VolEaseAddr          (DSP2PmAddr_treblescale48KAddr  + 1  )

        /* ATV_Enc */
            #define DSP2_PM_ATV_Enc_input_attenuation_ADDR   (DSP2PmAddr_alg1Ver+1)
            #define DSP2_PM_ATV_Enc_output_scaling_ADDR      (DSP2PmAddr_alg1Ver+2)
            #define DSP2_PM_BTSC_Enc_output_M_gain_ADDR      (DSP2PmAddr_alg1Ver+3)
            #define DSP2_PM_BTSC_Enc_output_D_gain_ADDR      (DSP2PmAddr_alg1Ver+4)
            #define DSP2_PM_BTSC_Enc_output_SAP_gain_ADDR    (DSP2PmAddr_alg1Ver+5)

    /* SND DSP DM vars */

        /* Extra Box Address */

            /* sys_param */
            #define DSP2XboxAddr_mainVer              0xB800
            #define DSP2XboxAddr_mainAudioDelay       (DSP2XboxAddr_mainVer +   0x01)
            #define DSP2XboxAddr_spdifDelay           (DSP2XboxAddr_mainVer +   0x02)
            #define DSP2XboxAddr_hpDelay              (DSP2XboxAddr_mainVer +   0x03)
            #define DSP2XboxAddr_hdmiNonPcmSts        (DSP2XboxAddr_mainVer +   0x04)
            #define DSP2XboxAddr_btFrameSize          (DSP2XboxAddr_mainVer +   0x05)
            #define DSP2XboxAddr_ipSecurity           (DSP2XboxAddr_mainVer +   0x06)
            #define DSP2XboxAddr_hdmiDelay            (DSP2XboxAddr_mainVer +   0x07)
            #define DSP2XboxAddr_ch5InputDelay        (DSP2XboxAddr_mainVer +   0x08)
            #define DSP2XboxAddr_ch6InputDelay        (DSP2XboxAddr_mainVer +   0x09)
            #define DSP2XboxAddr_multiChInputDelay    (DSP2XboxAddr_mainVer +   0x0A)
            #define DSP2XboxAddr_ipSecurity2          (DSP2XboxAddr_mainVer +   0x0B)

            #define DSP2XboxAddr_peq48KCoeffAddr      (DSP2XboxAddr_mainVer +   0x10)   // len 40
            #define DSP2XboxAddr_peq32KCoeffAddr      (DSP2XboxAddr_peq48KCoeffAddr)
            #define DSP2XboxAddr_peqscale48KAddr      (DSP2XboxAddr_mainVer +   0x38)   // len 8
            #define DSP2XboxAddr_peqbandEnAddr        (DSP2XboxAddr_mainVer +   0x40)   // len 1
            #define DSP2XboxAddr_peqbandDoubleAddr    (DSP2XboxAddr_mainVer +   0x41)   // len 1
            #define DSP2XboxAddr_hpf48KCoeffAddr      (DSP2XboxAddr_mainVer +   0x42)   // len 5
            #define DSP2XboxAddr_toneSelectAddr       (DSP2XboxAddr_mainVer +   0x47)   // len 1
            #define DSP2XboxAddr_bass48KCoeffAddr     (DSP2XboxAddr_mainVer +   0x48)   // len 5
            #define DSP2XboxAddr_bassscale48KAddr     (DSP2XboxAddr_mainVer +   0x4D)   // len 1
            #define DSP2XboxAddr_treble48KCoeffAddr   (DSP2XboxAddr_mainVer +   0x4E)   // len 5
            #define DSP2XboxAddr_treblescale48KAddr   (DSP2XboxAddr_mainVer +   0x53)   // len 1
            #define DSP2XboxAddr_VolEaseAddr          (DSP2XboxAddr_mainVer +   0x54)   // len 9

            #define DSP2XboxAddr_peqUpdateFlag        (DSP2XboxAddr_mainVer +   0x5D)   // len 1
            #define DSP2XboxAddr_hpUpdateFlag         (DSP2XboxAddr_mainVer +   0x5E)   // len 1
            #define DSP2XboxAddr_bassUpdateFlag       (DSP2XboxAddr_mainVer +   0x5F)   // len 1
            #define DSP2XboxAddr_trebleUpdateFlag     (DSP2XboxAddr_mainVer +   0x60)   // len 1
            #define DSP2XboxAddr_toneUpdateFlag       (DSP2XboxAddr_mainVer +   0x61)   // len 1
            #define DSP2XboxAddr_easeAUpdateFlag      (DSP2XboxAddr_mainVer +   0x62)   // len 1
            #define DSP2XboxAddr_easeBUpdateFlag      (DSP2XboxAddr_mainVer +   0x63)   // len 1

            #define DSP2XboxAddr_ATVEnc_input_attenuation_ADDR   (DSP2XboxAddr_mainVer +   0x64)    // len 1
            #define DSP2XboxAddr_ATVEnc_output_scaling_ADDR      (DSP2XboxAddr_ATVEnc_input_attenuation_ADDR + 1)
            #define DSP2XboxAddr_BTSCEnc_output_M_gain_ADDR      (DSP2XboxAddr_ATVEnc_output_scaling_ADDR + 1)
            #define DSP2XboxAddr_BTSCEnc_output_D_gain_ADDR      (DSP2XboxAddr_BTSCEnc_output_M_gain_ADDR + 1)
            #define DSP2XboxAddr_BTSCEnc_output_SAP_gain_ADDR    (DSP2XboxAddr_BTSCEnc_output_D_gain_ADDR + 1)

            /* sys_info */
            #define DSP2XboxAddr_IO_Info1                        0xB900
                #define IO_INFO1_DAC1_OUT                        0x0000
                #define IO_INFO1_DAC2_OUT                        0x0002
                #define IO_INFO1_DAC3_OUT                        0x0004
                #define IO_INFO1_DAC4_OUT                        0x0006
                #define IO_INFO1_IIS1_OUT                        0x0008
                #define IO_INFO1_SPDIF_OUT                       0x000A
                #define IO_INFO1_HDMI_OUT                        0x000C

            #define DSP2XboxAddr_IO_Info2                        DSP2XboxAddr_IO_Info1 + 0x000E
                #define IO_INFO2_MUL_CH1                         0x0000
                #define IO_INFO2_MUL_CH2                         0x0002
                #define IO_INFO2_MUL_CH3                         0x0004
                #define IO_INFO2_MUL_CH4                         0x0006
                #define IO_INFO2_RAW                             0x0008
                #define IO_INFO2_RAW_DELAY                       0x000A
                #define IO_INFO2_RAW_DELAY_SE                    0x000C
                #define IO_INFO2_SCART                           0x000E
                #define IO_INFO2_KTV                             0x0010
                #define IO_INFO2_MUL_CH6                         0x0012
                #define IO_INFO2_SPDIF_DATA                      0x0014
                #define IO_INFO2_RESERVED4                       0x0016
                #define IO_INFO2_SINTONE                         0x0018
                #define IO_INFO2_MUL_CH8                         0x001A
                #define IO_INFO2_GPA_FS                          0x001C
                #define IO_INFO2_GPB_FS                          0x001D
                #define IO_INFO2_GPC_FS                          0x001E
                #define IO_INFO2_ALSA_MODE                       0x001F


        /* common */
            #define DSP2XboxAddr_dec1_signal_energy              (DSP2XboxAddr_IO_Info2 + 0x0020)   // len 1
            #define DSP2XboxAddr_pcmCapture_volume               (DSP2XboxAddr_IO_Info2 + 0x0021)   // len 1
            #define DSP2XboxAddr_pcmCapture2_volume              (DSP2XboxAddr_IO_Info2 + 0x0022)   // len 1
            #define DSP2XboxAddr_pcmCapture3_volume              (DSP2XboxAddr_IO_Info2 + 0x0023)   // len 1

            #define DSP2XboxAddr_hdmi_npcm_lock                  (DSP2XboxAddr_IO_Info2 + 0x0035)   // len 1
            #define DSP2XboxAddr_hdmi_unstable_protect           (DSP2XboxAddr_IO_Info2 + 0x0036)   // len 1
            #define DSP2XboxAddr_hdmi_unstable_threshold         (DSP2XboxAddr_IO_Info2 + 0x0037)   // len 1
            #define DSP2XboxAddr_mips_crisis_flag                (DSP2XboxAddr_IO_Info2 + 0x0038)   // len 1

            /* basic sound effect */
            #define DSP2XboxAddr_AvcSOffsetAddr                  (DSP2XboxAddr_IO_Info2 + 0x003B)   // len 1
            #define DSP2XboxAddr_KTV_XAGain                      (DSP2XboxAddr_IO_Info2 + 0x003C)   // len 1
            #define DSP2XboxAddr_KTV_XBGain                      (DSP2XboxAddr_IO_Info2 + 0x003D)   // len 1
            #define DSP2XboxAddr_Multi_Channel_VOL               (DSP2XboxAddr_IO_Info2 + 0x003E)   // len 1
            #define DSP2XboxAddr_MixerGroup1_Ctrl_addr           (DSP2XboxAddr_IO_Info2 + 0x003F)   // len 1
            #define DSP2Xboxaddr_MixerGroup2_Ctrl_addr           (DSP2XboxAddr_IO_Info2 + 0x0040)   // len 1
            #define DSP2XboxAddr_sinetone_StartFreq_Addr         (DSP2XboxAddr_IO_Info2 + 0x0041)   // len 1
            #define DSP2XboxAddr_sinetone_EndFreq_Addr           (DSP2XboxAddr_IO_Info2 + 0x0042)   // len 1
            #define DSP2XboxAddr_sinetone_SweepStep_Addr         (DSP2XboxAddr_IO_Info2 + 0x0043)   // len 1
            #define DSP2XboxAddr_sinetone_Duration_Addr          (DSP2XboxAddr_IO_Info2 + 0x0044)   // len 1

/********************************************************************
*  Decoder default setting
********************************************************************/
    /* SIF DSP PM vars */
    /*
            #define ADDR_gain_base_2                  0x2521   //B860
            #define ADDR_thr_base_2                   0x2620
            #define ADDR_pfir_base_2                  0x2690
            //  for SIF BTSC DSP PM vars //
            #define BTSC_COMPILE_OPTION_Addr_2        0x25F1   // len 1
            #define BTSC_OUTPUT_GAIN_Addr_2           0x2621   // len 2
            #define BTSC_THRESHOLD_Addr_2             0x2623   // len 10
            #define MTS_OUTPUT_GAIN_Addr_2            0x2634   //len 6
            #define SIF_AGC_THRESHOLD_Addr_2          0x252D   //len 3

            /// PAL gain setting address
            #define ADDR_fm_stdM_gain_2               ADDR_gain_base_2           // len = 4
            #define ADDR_fm_stdX_gain_2               ADDR_fm_stdM_gain_2+4      // len = 4
            #define ADDR_nicam_gain_2                 ADDR_fm_stdX_gain_2+4      // len = 2
            #define ADDR_am_gain_2                    ADDR_nicam_gain_2+2        // len = 2
            #define ADDR_agc_gain_2                   ADDR_am_gain_2+2           // len = 24

            // PAL threshold setting address
            #define ADDR_a2_stdM_thr_2                ADDR_thr_base_2            // len = 15
            #define ADDR_a2_stdBG_thr_2               ADDR_a2_stdM_thr_2+15      // len = 15
            #define ADDR_a2_stdDK_thr_2               ADDR_a2_stdBG_thr_2+15         // len = 15
            #define ADDR_a2_stdI_thr_2                ADDR_a2_stdDK_thr_2+15     // len = 4
            #define ADDR_am_thr_2                     ADDR_a2_stdI_thr_2+4       // len = 3
            #define ADDR_hidev_stdM_thr_2             ADDR_am_thr_2+3            // len = 4
            #define ADDR_hidev_stdBG_thr_2            ADDR_hidev_stdM_thr_2+4    // len = 4
            #define ADDR_hidev_stdDK_thr_2            ADDR_hidev_stdBG_thr_2+4   // len = 4
            #define ADDR_hidev_stdI_thr_2             ADDR_hidev_stdDK_thr_2+4   // len = 4
            #define ADDR_nicam_stdBG_pherr_thr_2      ADDR_hidev_stdI_thr_2+4    //len = 3
            #define ADDR_nicam_stdI_pherr_thr_2       ADDR_nicam_stdBG_pherr_thr_2+3  // len = 3
            #define ADDR_a2_bg_nicam_fm_nsr_thr_2     0x246F     // len = 1
            #define ADDR_a2_dk_nicam_fm_nsr_thr_2     0x2470     // len = 1

            // pfir setting address
            #define ADDR_hidev_demfir_2               ADDR_pfir_base_2             // len = 15
            #define ADDR_fm_ch1_pfir_2                ADDR_hidev_demfir_2+16       // len = 30
            #define ADDR_fm_ch2_pfir_2                ADDR_fm_ch1_pfir_2+30        // len = 30
            #define ADDR_hidev_lv1_pfir_2             ADDR_fm_ch2_pfir_2+30        // len = 20
            #define ADDR_hidev_lv2_pfir_2             ADDR_hidev_lv1_pfir_2+20     // len = 20
            #define ADDR_hidev_lv3_pfir_2             ADDR_hidev_lv2_pfir_2+20     // len = 20

            // BTSC threshold setting address
            #define HIDEV_NSR_THRESHOLD_Addr_2        BTSC_THRESHOLD_Addr_2+10            // len 2
            #define BTSC_MONO_AMP_THRESHOLD_Addr_2    HIDEV_NSR_THRESHOLD_Addr_2+2        // len 2
            #define HIDEV_AMP_THRESHOLD_Addr_2        BTSC_MONO_AMP_THRESHOLD_Addr_2+2    // len 2

            #define BTSC_SAP_PHASE_DIFF_CLIP_THR_Addr_2    HIDEV_AMP_THRESHOLD_Addr_2+2   // len 1
            #define BTSC_PILOT_DEBOUNCE_THRESHOLD_Addr_2   MTS_OUTPUT_GAIN_Addr_2+6       // len 3
            #define BTSC_SAP_ON_DETECTION_DEBOUNCE_THRESHOLD_Addr_2 BTSC_PILOT_DEBOUNCE_THRESHOLD_Addr_2+3 // len 1
    */

/************************************************
*   Below is MailBox config
*************************************************/

    /************************************************
    *   MCU to DSP mailbox
    ************************************************/
    /* SIF */
    #define M2S_MBOX_SIF_CMD_STANDARD           MB_2DC0
    #define M2S_MBOX_SIF_CMD_PFIRBANDWIDTH      MB_2DC2
    #define M2S_MBOX_SIF_CMD_MODE1              MB_2DC4
    #define M2S_MBOX_SIF_CMD_MODE2              MB_2DC6

    /* ATV Encoder */
    #define M2S_MBOX_ATVEnc_MODE_CTRL                    MB_2DC0

    /* SPDIF */
    #define M2S_MBOX_SPDIF_SETTING                  MB_2D8E
    #define M2S_MBOX_HDMI_SETTING                   MB_2D8E
        #define MBOX_SPDIF_SETTING_BIT_MUTE             MBOX_BIT0
        #define MBOX_SPDIF_SETTING_BIT_NPCMSEL          MBOX_BIT1
        #define MBOX_SPDIF_SETTING_R2_NPCM_SELBIT       MBOX_BIT2
        #define MBOX_SPDIF_SETTING_DVB2_NPCM_SELBIT     MBOX_BIT3
        #define MBOX_SPDIF_SETTING_MINUS_11DB_BIT       MBOX_BIT5
        #define MBOX_HDMI_SETTING_BIT_NPCMSEL           MBOX_BIT13
        #define MBOX_HDMI_SETTING_BIT_HDMI_OUTPATH      MBOX_BIT14
        #define MBOX_HDMI_SETTING_BIT_MUTE              MBOX_BIT15

    #define M2S_MBOX_DOLBY_LOUDNESS_INFO            MB_2D98
        #define MBOX_DOLBY_LOUDNESS_ENABLE_BIT          MBOX_BIT15
        #define MBOX_DOLBY_LOUDNESS_ATSC_MODE           MBOX_BIT14
        #define MBOX_DOLBY_LOUDNESS_OTHER_MODE          MBOX_BIT13

    /* Sound effect */
    #define M2S_MBOX_AUOUT0_VOL                 MB_2D00
    #define M2S_MBOX_AUOUT1_VOL                 MB_2D02
    #define M2S_MBOX_AUOUT2_VOL                 MB_2D04
    #define M2S_MBOX_AUOUT3_VOL                 MB_2D06
    #define M2S_MBOX_I2S_VOL                    MB_2D08
    #define M2S_MBOX_SPDIF_VOL                  MB_2D0A
    #define M2S_MBOX_SRC_VOL                    MB_2D0C
    #define M2S_MBOX_HDMI_VOL                   MB_2D0E    //STB only
    #define M2S_MBOX_I2S2_VOL                   MB_2D0E
    #define M2S_MBOX_PRESCALE                   MB_2D10

    #define M2S_MBOX_EQ1_GAIN                   MB_2D14             //[15:8]
    #define M2S_MBOX_BASS_CTRL                  MB_2D14             //[7:0]
    #define M2S_MBOX_EQ2_GAIN                   MB_2D16             //[15:8]
    #define M2S_MBOX_TREBLE_CTRL                MB_2D16             //[7:0]
    #define M2S_MBOX_EQ3_GAIN                   MB_2D18             //[15:8]
    #define M2S_MBOX_SUPBASS_CTRL               MB_2D18             //[7:0]
    #define M2S_MBOX_EQ4_GAIN                   MB_2D1A             //[15:8]
    #define M2S_MBOX_EQ5_GAIN                   MB_2D1C             //[15:8]
    #define M2S_MBOX_BAL_CTRL                   MB_2D1E

    #define M2S_MBOX_SNDEFF_EN                  MB_2D20
        #define M2S_MBOX_STEREO_EN_BIT              MBOX_BIT15
        #define M2S_MBOX_DRC_EN_BIT                 MBOX_BIT13
        #define M2S_MBOX_AVC_EN_BIT                 MBOX_BIT12
        #define M2S_MBOX_TONE_EN_BIT                MBOX_BIT11
        #define M2S_MBOX_SPATIAL_EN_BIT             MBOX_BIT10
        #define M2S_MBOX_VOLBAL_EN_BIT              MBOX_BIT9
        #define M2S_MBOX_GEQ_EN_BIT                 MBOX_BIT7
        #define M2S_MBOX_EASE_EN_BIT                MBOX_BIT6
        #define M2S_MBOX_BASSBOOST_EN_BIT           MBOX_BIT5
        #define M2S_MBOX_ECHO_EN_BIT                MBOX_BIT4
        #define M2S_MBOX_DC_REMOVE_EN_BIT           MBOX_BIT3
        #define M2S_MBOX_HPF_EN_BIT                 MBOX_BIT2
        #define M2S_MBOX_COEFFUPDATE_EN_BIT         MBOX_BIT1
        #define M2S_MBOX_PEQ_EN_BIT                 MBOX_BIT0

    #define M2S_MBOX_VOLUME_EN                  MB_2D22
        #define M2S_MBOX_VOL_DAC0_EN_BIT            MBOX_BIT0
        #define M2S_MBOX_VOL_DAC1_EN_BIT            MBOX_BIT1
        #define M2S_MBOX_VOL_DAC2_EN_BIT            MBOX_BIT2
        #define M2S_MBOX_VOL_DAC3_EN_BIT            MBOX_BIT3
        #define M2S_MBOX_VOL_I2S0_EN_BIT            MBOX_BIT4
        #define M2S_MBOX_VOL_I2S1_EN_BIT            MBOX_BIT5
        #define M2S_MBOX_VOL_I2S2_EN_BIT            MBOX_BIT6
        #define M2S_MBOX_VOL_I2S3_EN_BIT            MBOX_BIT7
        #define M2S_MBOX_VOL_SPDIF_EN_BIT           MBOX_BIT8
        #define M2S_MBOX_VOL_SRC_EN_BIT             MBOX_BIT9

    #define M2S_MBOX_AVC_CTRL                   MB_2D24         //[15:13] AT, [12:10] RT, [9:8] AVC_Mode, [7:0] AVC Threshold
    #define M2S_MBOX_SURR_CTRL                  MB_2D26

    #define M2S_MBOX_SINE_GEN                   MB_2D28
        #define M2S_MBOX_SINE_GEN_CTRL_MASK         0x7F
        #define M2S_MBOX_SINE_GEN_EN_BIT            MBOX_BIT7
        #define M2S_MBOX_MENUSOUND_MODE_SEL_MASK    0xF0        // [7:4] MENUSOUND Mode_Select, 0: Sine tone (default 1khz), 1: Sweep, 2: Sweep_300_800_5sec, 3: Sweep_800_300_5sec
        #define M2S_MBOX_MENUSOUND_MUTE_RIGHT_BIT   MBOX_BIT3   // [3]   MENUSOUND RIGHT_CHANNEL_MUTE, 1->mute, 0->un-mute
        #define M2S_MBOX_MENUSOUND_MUTE_LEFT_BIT    MBOX_BIT2   // [2]   MENUSOUND LEFT_CHANNEL_MUTE , 1->mute, 0->un-mute
        #define M2S_MBOX_MENUSOUND_PAUSE_BIT        MBOX_BIT1   // [1]   MENUSOUND PAUSE
        #define M2S_MBOX_MENUSOUND_PLAY_BIT         MBOX_BIT0   // [0]   MENUSOUND PLAY Trigger bit when change (from 0 to 1)

    #define M2S_MBOX_BALANCE_EN                 MB_2D2A
        #define M2S_MBOX_BAL_DAC0_EN_BIT            MBOX_BIT0
        #define M2S_MBOX_BAL_DAC1_EN_BIT            MBOX_BIT1
        #define M2S_MBOX_BAL_DAC2_EN_BIT            MBOX_BIT2
        #define M2S_MBOX_BAL_DAC3_EN_BIT            MBOX_BIT3
        #define M2S_MBOX_BAL_I2S0_EN_BIT            MBOX_BIT4
        #define M2S_MBOX_BAL_I2S1_EN_BIT            MBOX_BIT5
        #define M2S_MBOX_BAL_I2S2_EN_BIT            MBOX_BIT6
        #define M2S_MBOX_BAL_I2S3_EN_BIT            MBOX_BIT7
        #define M2S_MBOX_BAL_SPDIF_EN_BIT           MBOX_BIT8
        #define M2S_MBOX_BAL_SRC_EN_BIT             MBOX_BIT9
        #define M2S_MBOX_BAL_HDMI_EN_BIT            MBOX_BIT10

    #define M2S_MBOX_DRC_CTRL                   MB_2D2E             //[7:0] DRC1 Threshold
    #define M2S_MBOX_SOUND_MODE_SEL             MB_2D30             //[1:0] : LR / LL / RR
    #define M2S_MBOX_POWER_DOWN                 MB_2D30             //[8:9]
        #define M2S_MBOX_MCU_UPDATEXBOX_BIT         MBOX_BIT15

    #define M2S_MBOX_NR_CTRL                    MB_2D32

    #define M2S_MBOX_ADVSND_EN                  MB_2D40
    	#define M2S_MBOX_ADVSND_SUMMARY_EN_BIT      MBOX_BIT15

    #define M2S_MBOX_KTV_CTRL                   MB_2D46
    #define M2S_MBOX_KTV_EN_BIT                 MBOX_BIT15

    #define M2S_MBOX_INPUT_MUX_SEL1             MB_2D50
    #define M2S_MBOX_INPUT_MUX_SEL2             MB_2D52
    #define M2S_MBOX_INPUT_MUX_SEL3             MB_2D54

    #define M2S_MBOX_KTV8_VOL                   MB_2D5A
    #define M2S_MBOX_KTV5_VOL                   MB_2D5C
    #define M2S_MBOX_KTV6_VOL                   MB_2D5E
    #define M2S_MBOX_CH7_VOL                    MB_2D58

    #define M2S_MBOX_AD_CONTROL                 MB_2DD8
        #define M2S_MBOX_MIX_MODE_BSTART            MBOX_BITS_SHIFT-11
        #define M2S_MBOX_MIX_MODE_BMASK             0x7

        /* M2S_MBOX_MIX_MODE */
        #define GPA_MIX_MODE_IS_FORWARD         0
        #define GPA_MIX_MODE_IS_BACKWARD        1
        #define GPA_MIX_MODE_IS_NULL            2      ! 2 & 3: NULL_Mixer_Mode

    #define M2S_MBOX_DBG_CMD1                   MB_2DDC
        #define MBOX_DBGCMD_SET_ADDR                0x0200
        #define MBOX_DBGCMD_WRITE_DM                0x0300
        #define MBOX_DBGCMD_WRITE_PM                0x0400
        #define MBOX_DBGCMD_READ_DM                 0x0500
        #define MBOX_DBGCMD_READ_PM                 0x0600
        #define MBOX_DBGCMD_READ_PMASK              0x0700
        #define MBOX_DBGCMD_READ_IMASK              0x0800

        #define MBOX_DBGCMD_READ_MAIN_VER           0x9000
        #define MBOX_DBGCMD_READ_ALG1_VER           0x9100
        #define MBOX_DBGCMD_READ_ALG2_VER           0x9200

        //#define MBOX_DBGCMD_MIP_INT                 0xE000
        //#define MBOX_DBGCMD_ENC_INT                 0xE100
        //#define MBOX_DBGCMD_FILE_PTS_INT            0xEA00

        #define MBOX_DBGCMD_RELOAD_SIF_BEG          0xF000
        #define MBOX_DBGCMD_RELOAD_SIF_END          0xF100
        #define MBOX_DBGCMD_WAIT_MCU_START          0xF300
        #define MBOX_DBGCMD_RELOAD_ADVSND_BEG       0xF400
        #define MBOX_DBGCMD_RELOAD_ADVSND_END       0xF500

    #define M2S_MBOX_DBG_CMD2                       MB_2DDE

    /* MISC */
    #define M2S_MBOX_SW_DMA_READER_DDR_WtPtr        MB_2D34
    #define M2S_MBOX_SW_DMA_READER_DDR_Ctrl         MB_2D36
    #define M2S_MBOX_SW_DMA_READER_DDR_SynthFreq    MB_2D56
    #define M2S_MBOX_SW_DMA_READER_DDR_SynthFreq_L  0     //Reserved

    #define M2S_MBOX_CAPTURE_CTRL               MB_2D4A   //[7:0] PCM_capture1  [15:8] PCM_capture2
    //#define M2S_MBOX_CAPTURE3_CTRL              //MB_2D4C   //[7:0] PCM_capture3  //RESERVED
        #define M2S_MBOX_GET_CH5                    1
        #define M2S_MBOX_GET_CH6                    2
        #define M2S_MBOX_GET_CH7                    3
        #define M2S_MBOX_GET_CH8                    4
        #define M2S_MBOX_GET_ADC1                   5
        #define M2S_MBOX_GET_ADC2                   6
        #define M2S_MBOX_GET_Raw_Delay_SE           7
        #define M2S_MBOX_GET_MIXER                  8
        #define M2S_MBOX_GET_Raw                    9
        #define M2S_MBOX_GET_CH1                    0xA
        #define M2S_MBOX_GET_CH2                    0xB
        #define M2S_MBOX_GET_CH3                    0xC
        #define M2S_MBOX_GET_CH4                    0xD
        #define M2S_MBOX_GET_DEBUG                  128

    #define M2S_MBOX_PCM_CAPTURE_DDR_RdPtr          MB_2DD4
    #define M2S_MBOX_PCM_CAPTURE_DDR_Size           MB_2DD6

    #define M2S_MBOX_PCM_CAPTURE2_DDR_RdPtr         MB_2D38
    #define M2S_MBOX_PCM_CAPTURE2_DDR_Size          MB_2D3A

    //#define M2S_MBOX_PCM_CAPTURE3_DDR_RdPtr         //MB_2D94   //RESERVED
    //#define M2S_MBOX_PCM_CAPTURE3_DDR_Size          //MB_2D96   //RESERVED

    /************************************************
    *   DSP to MCU mailbox
    ************************************************/
    #define S2M_MBOX_ES_MEMCNT                  MB_2D70
    #define S2M_MBOX_PCM_MEMCNT                 MB_2D72
    #define S2M_MBOX_MM_BROWSE_TIME             MB_2D74
    #define S2M_MBOX_MM_PTS_IN_SEC              MB_2D76
    #define S2M_MBOX_MM_PTS_IN_MSEC             MB_2D78
    #define S2M_MBOX_MM_PTS_HI                  MB_2D7A
    #define S2M_MBOX_MM_PTS_ME                  MB_2D7C
    #define S2M_MBOX_MM_PTS_LO                  MB_2D7E

    #define S2M_MBOX_DEC_STATUS                 MB_2DFA

    #define S2M_MBOX_SIF_DETECTION_RESULT       MB_2DE0
    #define S2M_MBOX_SIF_STATUS_INFO            MB_2DE2
    #define S2M_MBOX_SIF_STATUS_MODE1           MB_2DE4
    #define S2M_MBOX_SIF_STATUS_MODE2           MB_2DE6
    #define S2M_MBOX_SIF_STATUS_NICAM_INFO      MB_2DE8
    #define S2M_MBOX_SIF_STATUS_NICAM_PARITY_ERR_CNT    MB_2DEA

    #define S2M_MBOX_NR_STATUS                  MB_2DEE
    #define S2M_MBOX_BSND_STATUS                MB_2DEE
        #define MBOX_NR_WORKING_NOW                 MBOX_BIT0           // 1: NR working now            , 0 NR not working
        #define MBOX_TONE_FUNC_SELECT               MBOX_BIT1           // 0: EQ_Bass_Treble            , 1: Bass_Treble_old
        #define MBOX_PEQ_FUNC_SELECT                MBOX_BIT2           // 0: PEQ: single precision     , 1: double precision
        #define S2M_MBOX_DSP_UPDATEXBOX_BIT         MBOX_BIT15

    #define S2M_MBOX_MAIN_OVERFLOW_CNT          MB_2DF2                 //[15:8], full cnt of input SRAM buff2
    #define S2M_MBOX_MAIN_UNDERFLOW_CNT         MB_2DF2                 //[ 7:0], empty cnt of output SRAM buff1

    #define S2M_MBOX_ISR_CNTR                   MB_2DF6                 //[15:8]
    #define S2M_MBOX_INTR_CMDTYPE               MB_2DF6                 //[ 7:0]
        #define SE_DSP_INTR_CMD_MMES_NEED_DATA      0x0300
        #define SE_DSP_INTR_CMD_REPORT_PTS          0x0500
        #define SE_DSP_INTR_CMD_MMUNI_NEED_DATA     0x0600
        #define SE_DSP_INTR_CMD_VOIP                0x0900
        #define DSP_INTR_CMD_PCM_UPLOAD             0x3300
        #define DSP_INTR_CMD_PCM_DOWNLOAD           0xC000

    #define S2M_MBOX_WHILE1_CNTR                MB_2DF8                 //[ 7:0] Always in Low  Byte
    #define S2M_MBOX_TIMER_CNTR                 MB_2DF8                 //[15:8] Always in High Byte

    #define S2M_MBOX_DBG_RESULT1                MB_2DFC                 //
    #define S2M_MBOX_DSP_INIT_ACK               0x00E3

    #define S2M_MBOX_DBG_RESULT2                MB_2DFE                 //
    #define MBOX_DSP_RELOAD_ACK1                0x0033
    #define MBOX_DSP_RELOAD_ACK2                0x0077

    #define S2M_MBOX_SW_DMA_READER_DDR_Level    MB_2DE0

    #define S2M_MBOX_PCM_CAPTURE_DDR_WrPtr      MB_2DF0
    #define S2M_MBOX_PCM_CAPTURE2_DDR_WrPtr     MB_2DF4
    //#define S2M_MBOX_PCM_CAPTURE3_DDR_WrPtr     //MB_2D7E //RESERVED

#ifdef _COMPILE_DSP_
/************************************************
*   DSP ddr address mapping
*************************************************/
    /* DRAM Config */
        .const DSP2_TO_COMMON_DRAM_OFFSET           = (ASND_DSP_DDR_SIZE / BYTES_IN_MIU_LINE);

    /* SPDIF delay (GP C Bffer) */
        .const DSP2_SPDIF_DLY_DRAM_BASE             = (OFFSET_SPDIF_DLY_DRAM_BASE / BYTES_IN_MIU_LINE);
        .const DSP2_SPDIF_DLY_DRAM_SIZE             = ((SPDIF_DLY_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1);

    /* HDMI delay (GP C Bffer) */
        .const HDMI_DLY_DRAM_BASE                   = (SE_HDMI_DLY_DRAM_BASE / BYTES_IN_MIU_LINE);
        .const HDMI_DLY_DRAM_SIZE                   = (SE_HDMI_DLY_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1;

    /* sound system */
        .const DSP2_DMA_START_DRAM_BASE1            = (OFFSET_SE_MAIN_IN_DRAM_ADDR / BYTES_IN_MIU_LINE);
        .const DSP2_DMA_START_DRAM_SIZE1            = (SE_MAIN_IN_DRAM_SIZE / BYTES_IN_MIU_LINE);
        .const DSP2_DMA_START_DRAM_BASE2            = (OFFSET_SE_MAIN_OUT_DRAM_ADDR / BYTES_IN_MIU_LINE);
        .const DSP2_DMA_START_DRAM_SIZE2            = (SE_MAIN_OUT_DRAM_SIZE / BYTES_IN_MIU_LINE);

    /* Surround */
        #if(MSTAR_SURROUND_DRAM_SIZE>0)
        #define SUR_DRAM_BASEADDR                   (OFFSET_MSTAR_SURROUND_DRAM_ADDR / BYTES_IN_MIU_LINE)                               // Line Address
        #define SUR_DRAM_ENDADDR                    ((OFFSET_MSTAR_SURROUND_DRAM_ADDR + 0x0007E00) / BYTES_IN_MIU_LINE) // ((OFFSET_MSTAR_SURROUND_DRAM_ADDR + MSTAR_SURROUND_DRAM_SIZE) / BYTES_IN_MIU_LINE)  // Line Address
        #endif

    /* HE-AAC Metadata Buffer on DEC */
        .const DSP2_HEAAC_METADATA_DRAM_BASE        = (OFFSET_DDENC_METADATA_DRAM_ADDR / BYTES_IN_MIU_LINE);
        .const DSP2_HEAAC_METADATA_DRAM_SIZE        = (DDENC_METADATA_DRAM_SIZE / BYTES_IN_MIU_LINE);        // 8KB

    /* KTV */
        #if(KTV_SURROUND_DRAM_SIZE>0)
        #define SUR_DRAM_KTV_BASEADDR               (OFFSET_KTV_SURROUND_DRAM_ADDR / BYTES_IN_MIU_LINE)                             // Line address, Only in KTV mode, MS surround -> echo
        #define SUR_DARM_KTV_ENDADDR                ((OFFSET_KTV_SURROUND_DRAM_ADDR + 0x0001FE00) / BYTES_IN_MIU_LINE) //((OFFSET_KTV_SURROUND_DRAM_ADDR + KTV_SURROUND_DRAM_SIZE) / BYTES_IN_MIU_LINE)  // Line address, Overlay with DM prefetch
        #endif

    /* DSP DM Prefetch */
        #define DSP2_DM_PREFETCH_DRAM_BASE          (OFFSET_DM_PREFETCH_DRAM_ADDR / BYTES_IN_MIU_LINE)
        #define DSP2_DM_PREFETCH_DRAM_SIZE          (DM_PREFETCH_DRAM_SIZE / BYTES_IN_MIU_LINE)        // 64KB

    /* standalone DDCO PCM Buffer */
        //#define DSP2_DDE_PCM_DRAM_BASE              (DSP2_DM_PREFETCH_DRAM_BASE+0x400)      // AC3 Encode base address
        //#define DSP2_DDE_PCM_DRAM_SIZE              (0xBFF)           // 48KB
        .const DSP2_DDE_PCM_DRAM_BASE                = (OFFSET_SER2_DDENC_MCHOUT_DRAM_ADDR / BYTES_IN_MIU_LINE);      // AC3 Encode base address
        .const DSP2_DDE_PCM_DRAM_SIZE                = ((SER2_DDENC_MCHOUT_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1);       // 54kB

    /* HEAD PHONE delay */
        #if(HEAD_PHONE_DLY_DRAM_SIZE>0)
        .const DSP2_HEAD_PHONE_DLY_DRAM_BASE        = (OFFSET_HEAD_PHONE_DLY_DRAM_BASE / BYTES_IN_MIU_LINE); // Line address
        .const DSP2_HEAD_PHONE_DLY_DRAM_SIZE        = ((HEAD_PHONE_DLY_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1);
        #endif

    /* CH5 input delay */
        #if(CH5_INPUT_DLY_DRAM_SIZE>0)
        .const DSP2_CH5_INPUT_DLY_DRAM_BASE         = (OFFSET_CH5_INPUT_DLY_DRAM_BASE / BYTES_IN_MIU_LINE);  // Line address
        .const DSP2_CH5_INPUT_DLY_DRAM_SIZE         = ((CH5_INPUT_DLY_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1);
        #endif

    /* CH6 input delay */
        #if(CH6_INPUT_DLY_DRAM_SIZE>0)
        .const DSP2_CH6_INPUT_DLY_DRAM_BASE         = (OFFSET_CH6_INPUT_DLY_DRAM_BASE / BYTES_IN_MIU_LINE);  // Line address
        .const DSP2_CH6_INPUT_DLY_DRAM_SIZE         = ((CH6_INPUT_DLY_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1);
        #endif

    /* multiChInput audio delay */
        #if(MULTI_CH_INPUT_DLY_DRAM_SIZE>0)
        .const DSP2_MULTI_CH_INPUT_DLY_DRAM_BASE    = (OFFSET_MULTI_CH_INPUT_DLY_DRAM_BASE / BYTES_IN_MIU_LINE); // Line address
        .const DSP2_MULTI_CH_INPUT_DLY_DRAM_SIZE    = ((MULTI_CH_INPUT_DLY_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1);
        #endif

    /* SPDIF Non-PCM */
        .const DSP2_SPDIF_DRAM_BASE                 = OFFSET_SPDIF_NONPCM_DRAM_BASE / BYTES_IN_MIU_LINE;
        .const DSP2_SPDIF_DRAM_SIZE                 = (SPDIF_NONPCM_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1;

    /* HDMI Non-PCM */
        .const DSP2_HDMI_DRAM_BASE                  = OFFSET_HDMI_NONPCM_DRAM_BASE / BYTES_IN_MIU_LINE;
        .const DSP2_HDMI_DRAM_SIZE                  = (HDMI_NONPCM_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1;

    /* pcmR_dmxPcm from preAsndR2 */
        .const DSP2_PCMR_DMXPCM_DRAM_BASE           = (OFFSET_SER2_OUTPCM_DMX_DRAM_ADDR / BYTES_IN_MIU_LINE);
        .const DSP2_PCMR_DMXPCM_DRAM_SIZE           = ((SER2_OUTPCM_DMX_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1);

    /* COMMON DRAM */

        /* PCM 1 / 2 */
            .const DSP2_PCM1_DRAM_BASE              = (DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_PCM1_DRAM_ADDR / BYTES_IN_MIU_LINE));
            .const DSP2_PCM1_DRAM_SIZE              = (PCM1_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1;

            .const DSP2_PCM2_DRAM_BASE              = (DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_PCM2_DRAM_ADDR / BYTES_IN_MIU_LINE));
            .const DSP2_PCM2_DRAM_SIZE              = (PCM2_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1;

        /* Software DMA */
            #if(SW_DMA_READER_DRAM_SIZE>0)
            .const DSP2_SW_DMA_READER_DRAM_BASE     = (DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_SW_DMA_READER_DRAM_BASE / BYTES_IN_MIU_LINE));
            .const DSP2_SW_DMA_READER_DRAM_SIZE     = ((SW_DMA_READER_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1);
            #endif
        /* PCM capture buffer */
            #if(PCM_CAPTURE_BUFFER_DRAM_SIZE>0)
            .const DSP2_PCM_CAPTURE_BUFFER_DRAM_BASE    = DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_PCM_CAPTURE1_BUFFER_DRAM_BASE / BYTES_IN_MIU_LINE);
            .const DSP2_PCM_CAPTURE_BUFFER_DRAM_SIZE    = (PCM_CAPTURE_BUFFER_DRAM_SIZE/ BYTES_IN_MIU_LINE);
            #endif
            #if(PCM_CAPTURE2_BUFFER_DRAM_SIZE>0)
            .const DSP2_PCM_CAPTURE2_BUFFER_DRAM_BASE   = DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_PCM_CAPTURE2_BUFFER_DRAM_BASE / BYTES_IN_MIU_LINE);
            .const DSP2_PCM_CAPTURE2_BUFFER_DRAM_SIZE   = (PCM_CAPTURE2_BUFFER_DRAM_SIZE/ BYTES_IN_MIU_LINE);
            #endif
            #if(PCM_CAPTURE3_BUFFER_DRAM_SIZE>0)
            .const DSP2_PCM_CAPTURE3_BUFFER_DRAM_BASE   = DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_PCM_CAPTURE3_BUFFER_DRAM_BASE / BYTES_IN_MIU_LINE);
            .const DSP2_PCM_CAPTURE3_BUFFER_DRAM_SIZE   = (PCM_CAPTURE3_BUFFER_DRAM_SIZE/ BYTES_IN_MIU_LINE);
            #endif
            #define DSP2_PCM_CAPTURE_COPY_WORDSIZE      32
            #define DSP2_PCM_CAPTURE_COPY_LINESIZE      4

/************************************************
*  DSP TSCALE & TCOUNT setting
************************************************/
#define DSP_SYSTEM_FREQUENCY        432
#define TSCALE_CONSTANT             0xF9
#define DSP_TIME_CONSTANT           (DSP_SYSTEM_FREQUENCY/2 -1)
#define DSP_TIMER_SETTING           DSP_TIME_CONSTANT

/************************************************
*   Below is DMA config
*************************************************/
    #define DMAITF_DSPCMD_ALIGNMENT_BIT         7           // 1/0 : msb / lsb alignment
    #define DMAITF_DSPCMD_BYTESWAP_BIT          6           // set 1 to byte swap
    #define DMAITF_DSPCMD_READY_BIT             5           // set 1 to trigger, will be 0 when dma is finished
    #define DMAITF_DSPCMD_CLRCNTR_BIT           4           // set 1 to clear
    #define DMAITF_DSPCMD_PRIORITY_BIT          3           // 1/0 : high / low
    #define DMAITF_DSPCMD_24BITS_BIT            2           // 1/0 : 24bits / 16bits

    #define DMAITF_DSPCMD_ALIGNMENT_MASK        0x80
    #define DMAITF_DSPCMD_BYTESWAP_MASK         0x40
    #define DMAITF_DSPCMD_READY_MASK            0x20
    #define DMAITF_DSPCMD_CLRCNTR_MASK          0x10
    #define DMAITF_DSPCMD_PRIORITY_MASK         0x08
    #define DMAITF_DSPCMD_24BITS_MASK           0x04
    #define DMAITF_DSPCMD_BURST_6               0x03
    #define DMAITF_DSPCMD_BURST_3               0x02
    #define DMAITF_DSPCMD_BURST_2               0x01
    #define DMAITF_DSPCMD_BURST_1               0x00

    #if (MIU_128 == 1)
        /* 1 MIU Line = 128bit (16 bytes) */
        #define DMAITF_DSPWORDS_IN_1_LINE           8
        #define DMAITF_DSPWORDS_IN_1LINE_LOG2       3
        #define DMAITF_DSPWORDS_IN_3LINE_LOG2       4
        #define DMAITF_WR_BIT                       16
        #define DMAITF_DM_BIT                       15
        #define DMAITF_BYTES_IN_1MIU_LINE           BYTES_IN_MIU_LINE

        #define DMAITF_RD_PM_MASK                   0x000000
        #define DMAITF_WR_PM_MASK                   0x010000
        #define DMAITF_RD_DM_MASK                   0x008000
        #define DMAITF_WR_DM_MASK                   0x018000

        #define DMAITF_16BITS_B1_DMA_CMD            0xA8                // 16 Bits Burst 1, high alignment
        #define DMAITF_16BITS_B2_DMA_CMD            0xA8                //no Burst 2 cmd, use B1 instead
        #define DMAITF_16BITS_B1_SWAP_DMA_CMD       0xE8                // 16 Bits Burst 1, high alignment
        #define DMAITF_16BITS_B2_SWAP_DMA_CMD       0xE8                //no Burst 2 cmd, use B1 instead

        #define DMAITF_24BITS_B3_DMA_CMD            0x2E                // 24 Bits Burst 3
        #define DMAITF_24BITS_B6_DMA_CMD            0x2E                //no Burst 6 cmd, use B3 instead

        #define DMAITF_16BITS_LowAlign_B1_DMA_CMD   0x28                //16 Bits Burst 1, low alignment
        #define DMAITF_16BITS_LowAlign_B1_SWAP_DMA_CMD   0x68                //16 Bits Burst 1, low alignment
    #else
        /* 1 MIU Line = 64bit (8 bytes) */
        #define DMAITF_DSPWORDS_IN_1_LINE           4
        #define DMAITF_DSPWORDS_IN_1LINE_LOG2       2
        #define DMAITF_DSPWORDS_IN_3LINE_LOG2       3
        #define DMAITF_WR_BIT                       15
        #define DMAITF_DM_BIT                       14
        #define DMAITF_BYTES_IN_1MIU_LINE           BYTES_IN_MIU_LINE

        #define DMAITF_RD_PM_MASK                   0x000000
        #define DMAITF_WR_PM_MASK                   0x008000
        #define DMAITF_RD_DM_MASK                   0x004000
        #define DMAITF_WR_DM_MASK                   0x00C000

        #define DMAITF_16BITS_B1_DMA_CMD            0xA8                // 16 Bits Burst 1, high alignment
        #define DMAITF_16BITS_B2_DMA_CMD            0xA9
        #define DMAITF_16BITS_B1_SWAP_DMA_CMD       0xE8                // 16 Bits Burst 1, high alignment
        #define DMAITF_16BITS_B2_SWAP_DMA_CMD       0xE9

        #define DMAITF_24BITS_B3_DMA_CMD            0x2E                // 24 Bits Burst 3
        #define DMAITF_24BITS_B6_DMA_CMD            0x2F                // 24 Bits Burst 6

        #define DMAITF_16BITS_LowAlign_B1_DMA_CMD   0x28                //16 Bits Burst 1, low alignment
        #define DMAITF_16BITS_LowAlign_B1_SWAP_DMA_CMD   0x68                //16 Bits Burst 1, low alignment
    #endif

    /* DMA Mapping */
    #define General_DSP_IDMA_CMD_Number             15              // just info this chip nubmer of DSP_IDMA can support, (no any purpose) need sync with HW spec

        #define SNDISR1_DMA_CTRL                DSPDMA7_DMA_CTRL
        #define SNDISR2_DMA_CTRL                DSPDMA2_DMA_CTRL
        #define SPDIF_DMA_CTRL                  DSPDMA6_DMA_CTRL        // SPDIF npcm
        #define HDMI_DMA_CTRL                   DSPDMA6_DMA_CTRL        // HDMI npcm
        #define PCM_CAPTURE_DMA_CTRL            DSPDMA15_DMA_CTRL
        #define PCM_CAPTURE2_DMA_CTRL           DSPDMA14_DMA_CTRL
        #define PCM_CAPTURE3_DMA_CTRL           DSPDMA13_DMA_CTRL
        #define SW_DMARDR_DMA_CTRL              DSPDMA13_DMA_CTRL
        #define HP_DLY_DMA_CTRL                 DSPDMA13_DMA_CTRL
        #define CH5_IN_DLY_DMA_CTRL             DSPDMA12_DMA_CTRL
        #define CH6_IN_DLY_DMA_CTRL             DSPDMA11_DMA_CTRL
        #define MULTI_CH_INPUT_DLY_DMA_CTRL     DSPDMA9_DMA_CTRL
        #define R2_DEC_PCM1R_DMA_CTRL           PCM1R_DMA_CTRL
        #define DDE_ISR_PCM_DMA_CTRL            DSPDMA1_DMA_CTRL
        #define SPDIF_DLY_IN_DMA_CTRL           DSPDMA2_DMA_CTRL
        #define HDMI_DLY_IN_DMA_CTRL            DSPDMA2_DMA_CTRL
        #define SPDIF_DLY_OUT_DMA_CTRL          DSPDMA6_DMA_CTRL
        #define HDMI_DLY_OUT_DMA_CTRL           DSPDMA6_DMA_CTRL
        #define PCMR_DMXPCM_DMA_CTRL            PCM1R_DMA_CTRL

        #define SNDBG_DMA_CTRL                  DSPDMA3_DMA_CTRL        // Background sound effect
        #define ADEC_DMA1_CTRL                  DSPDMA3_DMA_CTRL
        #define ADEC_DMA2_CTRL                  DSPDMA4_DMA_CTRL
        #define ADEC_DMA3_CTRL                  DSPDMA3_DMA_CTRL
        #define ADEC_DMA4_CTRL                  DSPDMA4_DMA_CTRL
        #define ADEC_DMA5_CTRL                  DSPDMA5_DMA_CTRL

/************************************************
*   Below is DSP FIFO/DDR unit Setting
*************************************************/
        .const SE_R2_FRAME_SIZE                     = 256;    //256 samples for R2_SE
        .const SE_PROCESS_FRAME_SMP_UNIT            = 128;     //128 samples per frame

        .const SE_PROCESS_FETCH_FRAME_LINE_SIZE     = SE_PROCESS_FRAME_SMP_UNIT*SE_PROCESS_FETCH_CHANNELS*3/BYTES_IN_MIU_LINE;
        .const SE_PROCESS_STORE_FRAME_LINE_SIZE     = SE_PROCESS_FRAME_SMP_UNIT*SE_PROCESS_STORE_CHANNELS*3/BYTES_IN_MIU_LINE;

    /* sound effect buffer / share buffer setting */
        .const SE_PROCESS_FIFO_SIZE_UNIT            = 64;         //delay fifo size per channel
        .const SE_PROCESS_DMA_WORD_SIZE_UNIT        = 16;         //DMA_WORD_SIZE per channel

    /* input ISR PCM samples -> DDR1 unit setting */
        .const SE_BUFF2_FIFO_SIZE                   = (SE_PROCESS_FIFO_SIZE_UNIT * SE_PROCESS_FETCH_CHANNELS);
        .const SE_BUFF2_DMA_WORD_SIZE               = (SE_PROCESS_DMA_WORD_SIZE_UNIT * SE_PROCESS_FETCH_CHANNELS);    // (DMA_WORD_SIZE per channel) * channel number
        .const SE_BUFF2_DMA_24BIT_LINE_SIZE         = SE_BUFF2_DMA_WORD_SIZE*3/BYTES_IN_MIU_LINE;
        .const SE_BUFF2_DMA_16BIT_LINE_SIZE         = SE_BUFF2_DMA_WORD_SIZE*2/BYTES_IN_MIU_LINE;

    /* DDR2 --> output ISR PCM samples */
        .const SE_BUFF1_FIFO_SIZE                   = (SE_PROCESS_FIFO_SIZE_UNIT * SE_PROCESS_STORE_CHANNELS);
        .const SE_BUFF1_DMA_WORD_SIZE               = (SE_PROCESS_DMA_WORD_SIZE_UNIT * SE_PROCESS_STORE_CHANNELS);    // (DMA_WORD_SIZE per channel) * channel number
        .const SE_BUFF1_DMA_24BIT_LINE_SIZE         = SE_BUFF1_DMA_WORD_SIZE*3/BYTES_IN_MIU_LINE;
        .const SE_BUFF1_DMA_16BIT_LINE_SIZE         = SE_BUFF1_DMA_WORD_SIZE*2/BYTES_IN_MIU_LINE;

    /* share buffer in dm mapping */
        .const SE_PROCESS_BUFFER_MAIN               = 0x0;
        .const SE_PROCESS_BUFFER_MAIN_RAW1_LR       = SE_PROCESS_BUFFER_MAIN + SE_PROCESS_FRAME_SMP_UNIT*0;
        .const SE_PROCESS_BUFFER_MAIN_RAW2_LR       = SE_PROCESS_BUFFER_MAIN + SE_PROCESS_FRAME_SMP_UNIT*2;
        .const SE_PROCESS_BUFFER_MAIN_SE_LR         = SE_PROCESS_BUFFER_MAIN + SE_PROCESS_FRAME_SMP_UNIT*4;
        .const SE_PROCESS_BUFFER_MAIN_SE_LmRm       = SE_PROCESS_BUFFER_MAIN + SE_PROCESS_FRAME_SMP_UNIT*6;
        .const SURR_DLY_BUFFER                      = SE_PROCESS_BUFFER_MAIN + SE_PROCESS_FRAME_SMP_UNIT*8;
        .const NR_PARAMETER_BUFFER                  = SE_PROCESS_BUFFER_MAIN + SE_PROCESS_FRAME_SMP_UNIT*10;
        .const SE_PROCESS_BUFFER_MAIN_TMP           = SE_PROCESS_BUFFER_MAIN + SE_PROCESS_FRAME_SMP_UNIT*12;    //0x600

        #define DSP2DmAddr_system_shareBuff          0x0        //SE_PROCESS_BUFFER_MAIN
        #define DSP2DmAddr_system_shareBuff_size     (0x600)    //(SE_PROCESS_BUFFER_MAIN_TMP - SE_PROCESS_BUFFER_MAIN)
        #define DSP2DmAddr_advSnd_shareBuff_base     SE_PROCESS_BUFFER_MAIN_TMP
        #define DSP2DmAddr_advSnd_shareBuff_size     (0x1500)   // min size: (SE_PROCESS_FRAME_SMP_UNIT*SE_PROCESS_FETCH_CHANNELS) for system frame base SE

        .const SndEff_Array_TMP1                    = SE_PROCESS_BUFFER_MAIN_TMP;
        .const SndEff_Array_TMP2                    = SndEff_Array_TMP1 + SE_PROCESS_FRAME_SMP_UNIT*2;
        .const SndEff_Array_TMP3                    = SndEff_Array_TMP2 + SE_PROCESS_FRAME_SMP_UNIT*2;
        .const SndEff_Array_TMP4                    = SndEff_Array_TMP3 + SE_PROCESS_FRAME_SMP_UNIT*2;

        .const Apply_NR_Status_BUFFER               = NR_PARAMETER_BUFFER;
        .const Apply_NR_Gain_BUFFER                 = Apply_NR_Status_BUFFER + SE_PROCESS_FRAME_SMP_UNIT;

    /* General delay buffer template */
        .const DELAY_FIFO_SIZE                      = 128;
        .const DELAY_DMA_WORDSIZE                   = 32;
        .const DELAY_DMA_LINESIZE                   = DELAY_DMA_WORDSIZE*3/BYTES_IN_MIU_LINE;

        .const MULTI_CH_INPUT_DELAY_FIFO_SIZE       = (SE_PROCESS_FIFO_SIZE_UNIT * MULTI_CH_INPUT_DELAY_STORE_CHANNELS); // fifo len
        .const MULTI_CH_INPUT_DELAY_DMA_WORDSIZE    = (SE_PROCESS_DMA_WORD_SIZE_UNIT * MULTI_CH_INPUT_DELAY_STORE_CHANNELS);
        .const MULTI_CH_INPUT_DELAY_DMA_LINESIZE    = (MULTI_CH_INPUT_DELAY_DMA_WORDSIZE*3/BYTES_IN_MIU_LINE);

        /* Dly_status */
        .const DELAY_INPUT_STOP                     = 0;
        .const DELAY_OUTPUT_STOP                    = 1;
        .const DELAY_DLYIN_DMA_ASSERT               = 2;
        .const DELAY_DLYOUT_DMA_ASSERT              = 3;

    /* Mstar Surround delay buffer */
        .const SUR_FIFO_UNIT                        = SE_PROCESS_FRAME_SMP_UNIT*2;
        .const SUR_DRAM_BURSRT                      = (SUR_FIFO_UNIT*3)/BYTES_IN_MIU_LINE;

    /* spdif delay unit setting */
        .const SPDIF_DLYBUF_DMA_WORDSIZE          = 32;
        .const SPDIF_DLYBUF_DMA_LINESIZE          = SPDIF_DLYBUF_DMA_WORDSIZE*2/BYTES_IN_MIU_LINE;
        .const SPDIF_DLYFIFO_LEN                  = 64;

    /* spdif nonPcm unit setting */
        .const SPDIF_NPCM_DMA_WORDSIZE            = 32;
        .const SPDIF_NPCM_DMA_LINESIZE            = SPDIF_NPCM_DMA_WORDSIZE*2/BYTES_IN_MIU_LINE;
        .const SPDIF_NPCMFIFO_LEN                 = 64;

    /* pcmR_dmxPcm unit setting */
        .const PCMR_DMXPCM_DMA_WORDSIZE           = 32;
        .const PCMR_DMXPCM_DMA_LINESIZE           = PCMR_DMXPCM_DMA_WORDSIZE*2/BYTES_IN_MIU_LINE;
        .const PCMR_DMXPCMFIFO_LEN                = 64;

    /* hdmi npcm fifo unit setting */
        .const HDMI_NPCM_DMA_WORDSIZE             = 128;
        .const HDMI_NPCM_DMA_LINESIZE             = HDMI_NPCM_DMA_WORDSIZE*2/BYTES_IN_MIU_LINE;
        .const HDMI_NFIFO_LEN                     = 128*2;

    /* hdmi delay unit setting */
        .const HDMI_DLYBUF_DMA_WORDSIZE           = 32;
        .const HDMI_DLYBUF_DMA_LINESIZE           = HDMI_DLYBUF_DMA_WORDSIZE*2/BYTES_IN_MIU_LINE;
        .const HDMI_DLYFIFO_LEN                   = 64;

    /* SW DMA */
        .const SW_DMA_CTRL_RESET_BIT              = 0;
        .const SW_DMA_CTRL_START_BIT              = 1;
        .const SW_DMA_CTRL_CIRCL_BIT              = 2;

        .const SW_DMA_WORDSIZE                    = 32;
        .const SW_DMA_LINESIZE                    = SW_DMA_WORDSIZE*2/BYTES_IN_MIU_LINE;

/********************************************************************
* DSP ISR mapping
********************************************************************/
    /* default 0 (no use) */
    #define ISR_MASK_PCM                IMASK_SP1T_IRQ1
    #define ISR_MASK_PCM2               IMASK_SP1R_IRQ0
    #define ISR_MASK_TIMER              IMASK_TM

    /* default -1 (no use) */
    #define ISR_PMASK_PCM               -1
    #define ISR_PMASK_PCM2              -1
    #define ISR_PMASK_DEC_R2_CMD        20               //from R2_0 IO  0xB000_0860
    #define ISR_PMASK_R2_LOAD_CODE_CMD  21               //from R2_0 IO  0xB000_0840
    #define ISR_PMASK_SPDIF2_ISR        IMASK_IRQE1
    #define ISR_PMASK_HDMI_ISR          IMASK_IRQL1

    #define DEC_MAIN_FUNC_PTR           g_DecFunPtr
    #define PCMOUT_FUNC_PTR             g_IRQ1_isr_funcPtr
    #define SIF_PCMOUT_FUNC_PTR         g_IRQ0_isr_funcPtr
    #define SIF_ENC_FUNC_PTR            g_SifEncFuncPtr
    #define SIF_ENC_DATAOUT_FUNC_PTR    g_SifEncDataOutFuncPtr

/********************************************************************
* DSP internal mailbox mapping
********************************************************************/

    /* SPDIF NonPCM */
        #define D2S_MBOX_SPDIF_CTRL             ddco_spdifNpcmCtrl      // [0:2]=Acmod for AC3/AC3+/HE-AAC, [3]=LFE flag,
                                                                        // [ 8:11]: HDMI  sample rate
                                                                        // 0: 96K, 1: 88K, 2: 64K
                                                                        // 3: 48K, 4: 44K, 5: 32K
                                                                        // 6: 24K, 7: 22K, 8: 16K
                                                                        // 9: 12K, a: 11K, b:  8K
                                                                        // c:192K, d: 176K e: 128K
                                                                        // [12:15]: SPDIF sample rate
                                                                        // 0: 96K, 1: 88K, 2: 64K
                                                                        // 3: 48K, 4: 44K, 5: 32K
                                                                        // 6: 24K, 7: 22K, 8: 16K
                                                                        // 9: 12K, a: 11K, b:  8K
                                                                        // [18] info to DDCO: 1-> -4.75dB; 0-> do nothing
                                                                        // [19] : 1->48KHz, 0->non-48KHz
                                                                        // [20] MultiCH_EN, [21]=ADEC Stop/Play,[22]=Freeze,[23]=Start
        #define D2S_MBOX_SPDIF_WRPTR            ddco_spdifNpcmWrPtr
        #define S2D_MBOX_SPDIF_RDPTR            ddco_spdifNpcmRdPtr

        .const MBOX_MULTI_CHANNEL_ENABLE_BIT    = 20;
        .const MBOX_SPDIF_NPCM_CTRL_BIT_PLAY    = 17;
        .const MBOX_SPDIF_NPCM_CTRL_BIT_FREEZE  = 22;
        .const MBOX_SPDIF_NPCM_CTRL_BIT_START   = 19;
        .const MBOX_HDMI_NPCM_CTRL_BIT_PLAY    = 21;
        .const MBOX_HDMI_NPCM_CTRL_BIT_START   = 23;
            //[23]    HDMI nonPcm Start
            //[22]
            //[21]    HDMI nonPcm PlayEnable
            //[20]    HDMI HBR mode
            //[19]    SPDIF nonPcm Start
            //[18]    inform DDEncode to attenuate 4.75dB
            //[17]    SPDIF nonPcm PlayEnable
            //[16]
            //[15:12] SPDIF nonPcm sampleRate index
                // 0: 96K, 1: 88K, 2: 64K
                // 3: 48K, 4: 44K, 5: 32K
                // 6: 24K, 7: 22K, 8: 16K
                // 9: 12K, a: 11K, b:  8K

            //[11:8]  HDMI  nonPcm sampleRate index
                // 0: 96K, 1: 88K, 2: 64K
                // 3: 48K, 4: 44K, 5: 32K
                // 6: 24K, 7: 22K, 8: 16K
                // 9: 12K, a: 11K, b:  8K
                // c:192K, d: 176K e: 128K

            //[7]     HDMI is Pcm or nonPcm
            //[6]     SPDIF is Pcm or nonPcm
            //[5:4]   hdmi  nonPcm owner
            //[3:2]   spdif nonPcm owner
            //[0:1]   spdif/hdmi PCM attenuator index

        #define NULL_PAYLOAD_TEST               1

        #if (NULL_PAYLOAD_TEST == 1)
            .const SPDIF_NPCM_MUTE_FRMCNT       =   0x8;
            .const SPDIF_PCM_MUTE_SMPCNT        =   0;
            .const SPDIF_NPCM_NULL_FRMCNT       =   0x0 + SPDIF_NPCM_MUTE_FRMCNT;
        #else
            .const SPDIF_NPCM_MUTE_FRMCNT       =   0x10;
            .const SPDIF_PCM_MUTE_SMPCNT        =   0;
        #endif

    /* Mailbox with DEC R2 */
        #define D2S_MBOX_LOAD_CODE_CMD          DECR2M_2_DSP_MAILBOX0
        #define D2S_MBOX_PCMISR_CTRL            DECR2M_2_DSP_MAILBOX1
            #define MBOX_R2_PCM1ISR_PLAY_START_BIT          0           //--> 這各由 playSmpFlag / stop / pause 決定
            #define MBOX_R2_PCM1ISR_PLAY_MUTE_BIT           1           //--> Mute
            #define MBOX_R2_PCM1ISR_USING_ASINK_ISR_BIT     2
            #define MBOX_R2_PCM2ISR_PLAY_START_BIT          8
            #define MBOX_R2_PCM2ISR_PLAY_MUTE_BIT           9
            #define MBOX_R2_PCM2ISR_USING_ASINK_ISR_BIT     10

        #define D2S_MBOX_PCM1_DRAM_WRPTR        DECR2M_2_DSP_MAILBOX2
        #define D2S_MBOX_PCM1_SYNTH_H           DECR2M_2_DSP_MAILBOX3
        #define D2S_MBOX_PCM1_SYNTH_L           DECR2M_2_DSP_MAILBOX4

        #define D2S_MBOX_PCM2_DRAM_WRPTR        DECR2M_2_DSP_MAILBOX5
        #define D2S_MBOX_PCM2_SYNTH_H           DECR2M_2_DSP_MAILBOX6
        #define D2S_MBOX_PCM2_SYNTH_L           DECR2M_2_DSP_MAILBOX7

        #define D2S_MBOX_R2_TO_DSP_COMMAND      DECR2M_2_DSP_MAILBOX8
            #define D2S_CMD_UPD_PCM1_MUTECNT        0x0001      // Bit0
            #define D2S_CMD_CLR_PCM1_PLAYCNT        0x0002      // Bit1
            #define D2S_CMD_UPD_PCM1_PLAYCNT        0x0004      // Bit2
            #define D2S_CMD_FLUSH_PCM1_SMPS         0x0008      // Bit3
            #define D2S_CMD_RESET_PCM1              0x0010      // Bit4
            #define D2S_CMD_UPD_PCM2_MUTECNT        0x0020      // Bit5
            #define D2S_CMD_CLR_PCM2_PLAYCNT        0x0040      // Bit6
            #define D2S_CMD_UPD_PCM2_PLAYCNT        0x0080      // Bit7
            #define D2S_CMD_FLUSH_PCM2_SMPS         0x0100      // Bit8
            #define D2S_CMD_RESET_PCM2              0x0200      // Bit9

        #define D2S_MBOX_R2_TO_DSP_PARAM        DECR2M_2_DSP_MAILBOX9
        #define D2S_R2_SPDIF_WR_PTR             DECR2M_2_DSP_MAILBOXA
        #define D2S_MBOX_HDMI_NPCM_WRPTR        DECR2M_2_DSP_MAILBOXB

        #define D2S_MBOX_HDMI_NPCM_CMD          DECR2M_2_DSP_MAILBOXC
        #define D2S_R2_SPDIF_CTRL               DECR2M_2_DSP_MAILBOXC
        #define D2S_MBOX_NPCM_CTRL              D2S_R2_SPDIF_CTRL
            //#define MBOX_HDMI_NPCM_CTRL_BIT_START         MBOX_BIT23
            //#define MBOX_HDMI_NPCM_CTRL_BIT_PLAY          MBOX_BIT21
            //#define MBOX_MULTI_CHANNEL_ENABLE_BIT         MBOX_BIT20
            //#define MBOX_SPDIF_NPCM_CTRL_BIT_START        MBOX_BIT19
            #define MBOX_SPDIF_NPCM_DDE_MINUS_4_75DB      MBOX_BIT18
            //#define MBOX_SPDIF_NPCM_CTRL_BIT_PLAY         MBOX_BIT17
            #define MBOX_HDMI_NONPCM_FROM_ASND_DSP_BIT    MBOX_BIT5
            #define MBOX_HDMI_NONPCM_FROM_ASND_R2_BIT     MBOX_BIT4
            #define MBOX_SPDIF_NONPCM_FROM_ASND_DSP_BIT   MBOX_BIT3
            #define MBOX_SPDIF_NONPCM_FROM_ASND_R2_BIT    MBOX_BIT2
            //[23]    HDMI nonPcm Start
            //[22]
            //[21]    HDMI nonPcm PlayEnable
            //[20]    HDMI HBR mode
            //[19]    SPDIF nonPcm Start
            //[18]    inform DDEncode to attenuate 4.75dB
            //[17]    SPDIF nonPcm PlayEnable
            //[16]
            //[15:12] SPDIF nonPcm sampleRate index
                // 0: 96K, 1: 88K, 2: 64K
                // 3: 48K, 4: 44K, 5: 32K
                // 6: 24K, 7: 22K, 8: 16K
                // 9: 12K, a: 11K, b:  8K

            //[11:8]  HDMI  nonPcm sampleRate index
                // 0: 96K, 1: 88K, 2: 64K
                // 3: 48K, 4: 44K, 5: 32K
                // 6: 24K, 7: 22K, 8: 16K
                // 9: 12K, a: 11K, b:  8K
                // c:192K, d: 176K e: 128K

            //[7]     HDMI is Pcm or nonPcm
            //[6]     SPDIF is Pcm or nonPcm
            //[5:4]   hdmi  nonPcm owner
            //[3:2]   spdif nonPcm owner
            //[0:1]   spdif/hdmi PCM attenuator index   //Dolby Bulletin 11: PCM Level control, 0: 0dB,  1:-7dB(ATSC), 2:-8dB(DVB), 3:-11dB(ISDB)

        #define D2S_R2_DOLBY_META_DATA          DECR2M_2_DSP_MAILBOXD
        #define D2S_DSP_ENCODE_SETTING          DECR2M_2_DSP_MAILBOXE
             #define MBOX_NONPCM_DDE_ENABLE_BIT     MBOX_BIT6
             #define MBOX_NONPCM_DTSE_ENABLE_BIT    MBOX_BIT5
            //[6] DSP DDENC ENABLE bit
            //[5] DSP DTS ENC ENABLE bit
            //[4] LFE
            //[3:0] AC mode

        /************************************************************/

        #define S2D_MBOX_DSP_TO_R2_COMMAND      DSP_2_DECR2M_MAILBOX0
            #define S2D_CMD_RESET_PCM1_AVSYNC  0x0001
            #define S2D_CMD_RESET_PCM2_AVSYNC  0x0002

        #define S2D_MBOX_DSP_TO_R2_PARAM        DSP_2_DECR2M_MAILBOX1
        #define S2D_MBOX_R2CMD_RECEIVE_CNT      DSP_2_DECR2M_MAILBOX2

        #define S2D_MBOX_PCM1_PLAYCNT           DSP_2_DECR2M_MAILBOX3
        #define S2D_MBOX_PCM1_FIFOCNT           DSP_2_DECR2M_MAILBOX4
        #define S2D_MBOX_PCM1_DRAM_RDPTR        DSP_2_DECR2M_MAILBOX5

        #define S2D_MBOX_PCM2_PLAYCNT           DSP_2_DECR2M_MAILBOX6
        #define S2D_MBOX_PCM2_FIFOCNT           DSP_2_DECR2M_MAILBOX7
        #define S2D_MBOX_PCM2_DRAM_RDPTR        DSP_2_DECR2M_MAILBOX8

        #define S2D_MBOX_ENCODE_SURPPORT        DSP_2_DECR2M_MAILBOX9
            #define DDE_ENCODE_SURPPORT_BIT             MBOX_BIT0
            #define DTSE_ENCODE_SURPPORT_BIT            MBOX_BIT1
            #define DDPE_ENCODE_SURPPORT_BIT            MBOX_BIT2

        #define S2D_R2_SPDIF_RD_PTR             DSP_2_DECR2M_MAILBOXA
        #define S2D_MBOX_HDMI_NPCM_RDPTR        DSP_2_DECR2M_MAILBOXB

        #define S2D_IP_SECURITY_KEY2            DSP_2_DECR2M_MAILBOXC
        #define S2D_IP_SECURITY_KEY             DSP_2_DECR2M_MAILBOXE
        #define S2D_OTP_BOUNDING                DSP_2_DECR2M_MAILBOXF


        #define S2A_IP_SECURITY_KEY2            DSP_2_SNDR2M_MAILBOXD
        #define S2A_IP_SECURITY_KEY             DSP_2_SNDR2M_MAILBOXE
        #define S2A_OTP_BOUNDING                DSP_2_SNDR2M_MAILBOXF

        !#define S2A_R2_PCMIN_WRPTR              DSP_2_DECR2M_MAILBOX7
        !#define S2A_R2_PCMIN2_WRPTR             DSP_2_DECR2M_MAILBOX8
        !#define S2A_R2_SPDIF_RD_PTR             DSP_2_DECR2M_MAILBOX9
        !#define S2A_IP_SECURITY_KEY             DSP_2_DECR2M_MAILBOXE
        !#define S2A_OTP_BOUNDING                DSP_2_DECR2M_MAILBOXF

        !#define A2S_R2_PCMOUT_WRPTR             DECR2M_2_DSP_MAILBOXD
        !#define A2S_R2_PCMOUT2_WRPTR            DECR2M_2_DSP_MAILBOX0
        !#define A2S_R2_SPDIF_WR_PTR             DECR2M_2_DSP_MAILBOXE
        !#define A2S_R2_SPDIF_CTRL               DECR2M_2_DSP_MAILBOXF
        /* Mailbox with SND R2 */
        #define S2A_R2_PCMIN_WRPTR              DSP_2_SNDR2M_MAILBOX1
        #define S2A_R2_PCMIN2_WRPTR             DSP_2_SNDR2M_MAILBOX2
        #define S2A_R2_SPDIF_RD_PTR             DSP_2_SNDR2M_MAILBOX3
        #define S2A_MBOX_HDMI_NPCM_RDPTR        DSP_2_SNDR2M_MAILBOX4
        #define S2A_MBOX_PCMR_DMXPCM_RDPTR      DSP_2_SNDR2M_MAILBOX5  // pcmR_dmxPcm from preAsndR2 RDPTR
        #define S2A_MBOX_SPEAKER_CH_VOLUME      DSP_2_SNDR2M_MAILBOXA

        #define A2S_R2_PCMOUT_WRPTR             SNDR2M_2_DSP_MAILBOX1
        #define A2S_R2_PCMOUT2_WRPTR            SNDR2M_2_DSP_MAILBOX2
        #define A2S_R2_SPDIF_WR_PTR             SNDR2M_2_DSP_MAILBOX3
        #define A2S_MBOX_HDMI_NPCM_WRPTR        SNDR2M_2_DSP_MAILBOX4
        #define A2S_R2_SPDIF_CTRL               SNDR2M_2_DSP_MAILBOX5
        #define A2S_MBOX_HDMI_NPCM_CMD          SNDR2M_2_DSP_MAILBOX5
        #define A2S_R2_DOLBY_META_DATA          SNDR2M_2_DSP_MAILBOX6
        #define A2S_MBOX_DDENC_OUTMCH_WRPTR     SNDR2M_2_DSP_MAILBOX6

        #define A2S_MBOX_R2_TO_DSP_CTRL         SNDR2M_2_DSP_MAILBOX7
            #define A2S_DDENC_ENABLE                    MBOX_BIT0
            #define A2S_CMD_SOUND_MIXER_DISABLE_BIT     MBOX_BIT1       //[1] SOUND_MIXER, 0:in DSP (MS11), 1:in SND_R2 (MS12)

        #define A2S_MBOX_PCMR_DMXPCM_WRPTR      SNDR2M_2_DSP_MAILBOX9  // pcmR_dmxPcm from preAsndR2 WRPTR

        #define A2S_R2_SOUND_PROCESS_CTRL       SNDR2M_2_DSP_MAILBOXF
            #define MBOX_PRE_SOUND_PROCESS_DISABLE_BIT	    MBOX_BIT0
            #define MBOX_POST_SOUND_PROCESS_DISABLE_BIT	    MBOX_BIT1

/********************************************************************
* DSP io mapping
********************************************************************/
    #define NULL_IO                 0

    /* DSP common IO */
        #define DSPIO_SPDIF_IN_FREQ        STATUS_SPDIF_FREQ
        #define DSPIO_HDMI_IN_FREQ         STATUS_HDMI_FREQ
        #define DSPIO_HDMI_IN_PC           STATUS_HDMI_PC

    /* DSP Bounding IO */
        #define DSP_BOUND_OPTION            1
        #define DSPIO_BOUND_OPTION          0xA0FF
            .const BOUNDING_BIT_DD              = 0;                   //DD
            .const BOUNDING_BIT_DDP             = 1;                   //DD+
            .const BOUNDING_BIT_DPULSE          = 2;                   //Dolby Pulse (MS10 DDT) or DDCO
            .const BOUNDING_BIT_DVOL            = 3;                   //Dolby Volume
            .const BOUNDING_BIT_DTRUEHD         = 4;                   //Dolby TrueHD
            .const BOUNDING_BIT_DDDCO           = 5;                   //Dolby DDCO
            .const BOUNDING_BIT_DTSENVELO       = 6;                   //DTS Envelo / Symmetry
            .const BOUNDING_BIT_DTSDMP          = 7;                   //DTS DMP
            .const BOUNDING_BIT_DTSLBR          = 8;                   //DTS LBR
            .const BOUNDING_BIT_DTSTS           = 9;                   //DTS Transcoder
            .const BOUNDING_BIT_DTSCORELESS     = 10;                  //DTS Coreless
            .const BOUNDING_BIT_SRS             = 11;                  //SRS / Adv Sound
            .const BOUNDING_BIT_DOLBY           = 12;                  // when bit[12] = 0, all dolby ip's licenses open

        //#define AUTH_OPTION                   0x0FF2
            .const AUTH_BIT_DD                  = 0;
            .const AUTH_BIT_DDP                 = 1;
            .const AUTH_BIT_DDE                 = 2;
            .const AUTH_BIT_DTSDEC              = 3;
            .const AUTH_BIT_MS10DDT             = 4;
            .const AUTH_BIT_WMA                 = 5;
            .const AUTH_BIT_DRA                 = 6;
            .const AUTH_BIT_DTSLBR              = 7;
            .const AUTH_BIT_GAAC                = 8;
            .const AUTH_BIT_MS11DDT             = 9;
            .const AUTH_BIT_DEMOMODE            = 12;
            .const AUTH_BIT_COOK                = 16;
            .const AUTH_BIT_DTS_HD              = 17;
            .const AUTH_BIT_MS12_LC             = 18;
            .const AUTH_BIT_MS12_D              = 19;
            .const AUTH_BIT_SONICMOTION_ABS3D   = 20;
            .const AUTH_BIT_MS12_B              = 21;
            .const AUTH_BIT_DV258               = 22;

    /* IP AUTH */
        #define D2S_MBOX_IP_AUTH                DEC2SE_MAILBOX7

    /* PCM output port */
        #define SIF_DSP_MAIN_DMX_L_OUT          DEC4_PCM1_OUT
        #define SIF_DSP_MAIN_DMX_R_OUT          DEC4_PCM2_OUT

        !#define R2_PCM1_DMX__L_OUT              DEC3_PCM1_OUT       // channel mapping of PCM buffer, replace R2_DMA_READER1 (R2_DECODER1_OUTPUT_BY_DSP)
        !#define R2_PCM1_DMX__R_OUT              DEC3_PCM2_OUT
        !#define R2_PCM1_MCH__L_OUT              DEC3_PCM3_OUT
        !#define R2_PCM1_MCH__C_OUT              DEC3_PCM4_OUT
        !#define R2_PCM1_MCH__R_OUT              DEC3_PCM5_OUT
        !#define R2_PCM1_MCH_LS_OUT              DEC3_PCM6_OUT
        !#define R2_PCM1_MCH_RS_OUT              DEC3_PCM7_OUT
        !#define R2_PCM1_MCH_SW_OUT              DEC3_PCM8_OUT
        !#define R2_PCM1_SYNTH_L                 DVB3_FIX_SYNTH_NF_L
        !#define R2_PCM1_SYNTH_H                 DVB3_FIX_SYNTH_NF_H

        #define R2_PCM2_DMX__L_OUT              DEC4_PCM1_OUT       // channel mapping of PCM buffer, replace R2_DMA_READER2 (R2_DECODER2_OUTPUT_BY_DSP)
        #define R2_PCM2_DMX__R_OUT              DEC4_PCM2_OUT
        !#define R2_PCM2_MCH__L_OUT              DEC4_PCM3_OUT
        !#define R2_PCM2_MCH__C_OUT              DEC4_PCM4_OUT
        !#define R2_PCM2_MCH__R_OUT              DEC4_PCM5_OUT
        !#define R2_PCM2_MCH_LS_OUT              DEC4_PCM6_OUT
        !#define R2_PCM2_MCH_RS_OUT              DEC4_PCM7_OUT
        !#define R2_PCM2_MCH_SW_OUT              DEC4_PCM8_OUT
        #define R2_PCM2_SYNTH_L                 DVB4_FIX_SYNTH_NF_L
        #define R2_PCM2_SYNTH_H                 DVB4_FIX_SYNTH_NF_H

        #define DSP_SW_DMA_DMX_L_OUT            DEC5_PCM1_OUT
        #define DSP_SW_DMA_DMX_R_OUT            DEC5_PCM2_OUT
        #define DSP_SW_DMA_RDR_SYNTH_L          DVB5_FIX_SYNTH_NF_L     // SW DMA RDR use DVB5_SYNTH, MCU control, 0x112C28: Synth_L  0x112C26: Synth_H, Toggle 0x112C24[12] to update
        #define DSP_SW_DMA_RDR_SYNTH_H          DVB5_FIX_SYNTH_NF_H

/************************************************
*   Below is macro for DSP code only
*************************************************/
        #define DSP_DMA_CHECK

        #define INC_WHILE_ONE_CNTR              ar = dm (S2M_MBOX_WHILE1_CNTR); \
                                                ay0 = 0x00FF00; \
                                                af = ar and ay0;    \
                                                ar = ar + 0x000001; \
                                                ay0 = 0x0000FF; \
                                                ar = ar and ay0;    \
                                                ar = ar or af;  \
                                                dm (S2M_MBOX_WHILE1_CNTR) = ar

        #define INC_DEBUG_CNT(x)                ar = dm(kh_debugCnt+x);     \
                                                ar = ar + 1;    \
                                                dm(kh_debugCnt+x) = ar;

        #define CONFIG_PCM_OUTPUT_PORT          ar = 0;   \
                                                dm (DEC_OUT_SEL) = ar;

        #define TRIGGER_INT_TO_MCU              ar = 0x0000; IO(PDATA) = ar;   \
                                                nop; nop; nop; nop;   \
                                                nop; nop; nop; nop;   \
                                                ar = 0x8000; IO(PDATA) = ar;   \
                                                nop; nop; nop; nop;   \
                                                nop; nop; nop; nop;   \
                                                ar = 0x0000; IO(PDATA) = ar

        /* Saft jump to i0 ~ i7 x:address, y:i0 ~ i7 */
        #define I_REGISTER_JUMP(x,y)            sr = lshift x by -16(lo);   \
                                                y = x;    \
                                                CPR = sr0;  \
                                                jump (y);

        /* Saft call to i0 ~ i7 x:address, y:i0 ~ i7 */
        #define I_REGISTER_CALL(x,y)            sr = lshift x by -16(lo);   \
                                                y = x;    \
                                                CPR = sr0;  \
                                                call (y);

        #define SEND_INT_TO_R2(cmd, param)      ar = param;     \
                                                dm(S2D_MBOX_DSP_TO_R2_PARAM) = ar;      \
                                                ar = cmd;   \
                                                dm(S2D_MBOX_DSP_TO_R2_COMMAND) = ar

#else

        #define DSP2_TO_COMMON_DRAM_OFFSET          (ASND_DSP_DDR_SIZE / BYTES_IN_MIU_LINE)

        /* DMA Reader Buffer */
        #define DSP2_DMA_READER_DRAM_BASE           (DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_DMA_READER_DRAM_BASE / BYTES_IN_MIU_LINE))
        #define DSP2_DMA_READER_DRAM_SIZE           ((DMA_READER_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1)

        /* HW DMA Reader2 Buffer */
        #define DSP2_HW_DMA_READER2_DRAM_BASE       (DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_HW_DMA_READER2_DRAM_BASE /BYTES_IN_MIU_LINE))
        #define DSP2_HW_DMA_READER2_DRAM_SIZE       ((HW_DMA_READER2_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1)

        /* Software DMA */
        #define DSP2_SW_DMA_READER_DRAM_BASE        (DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_SW_DMA_READER_DRAM_BASE / BYTES_IN_MIU_LINE))
        #define DSP2_SW_DMA_READER_DRAM_SIZE        ((SW_DMA_READER_DRAM_SIZE / BYTES_IN_MIU_LINE) - 1)

        /* PCM capture buffer */
        #define DSP2_PCM_CAPTURE_BUFFER_DRAM_BASE   (DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_PCM_CAPTURE1_BUFFER_DRAM_BASE / BYTES_IN_MIU_LINE))
        #define DSP2_PCM_CAPTURE2_BUFFER_DRAM_BASE  (DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_PCM_CAPTURE2_BUFFER_DRAM_BASE / BYTES_IN_MIU_LINE))
        #define DSP2_PCM_CAPTURE3_BUFFER_DRAM_BASE  (DSP2_TO_COMMON_DRAM_OFFSET + (OFFSET_PCM_CAPTURE3_BUFFER_DRAM_BASE / BYTES_IN_MIU_LINE))
        #define DSP2_PCM_CAPTURE_BUFFER_DRAM_SIZE   (PCM_CAPTURE_BUFFER_DRAM_SIZE/ BYTES_IN_MIU_LINE)
        #define DSP2_PCM_CAPTURE2_BUFFER_DRAM_SIZE  (PCM_CAPTURE2_BUFFER_DRAM_SIZE/ BYTES_IN_MIU_LINE)
        #define DSP2_PCM_CAPTURE3_BUFFER_DRAM_SIZE  (PCM_CAPTURE3_BUFFER_DRAM_SIZE/ BYTES_IN_MIU_LINE)
        #define DSP2_PCM_CAPTURE_COPY_WORDSIZE      32
        #define DSP2_PCM_CAPTURE_COPY_LINESIZE      4

#endif //_COMPILE_DSP_

        //Reseved XBox
        /* srs puresound */
        //reserved XBox for SRS start from XBox 0xB960
            #define DSP2XboxAddr_SRS_PURESOUND_RESERVED1         0xB960

            #define SRS_PURESOUND_AeqFir_NumOfTaps_addr          DSP2XboxAddr_SRS_PURESOUND_RESERVED1+1
            #define SRS_PURESOUND_AeqIir_NumOfSections_addr      DSP2XboxAddr_SRS_PURESOUND_RESERVED1+2
            #define SRS_PURESOUND_AeqIir1Coefs_addr              DSP2XboxAddr_SRS_PURESOUND_RESERVED1+3
            #define SRS_PURESOUND_AeqIir2Coefs_addr              DSP2XboxAddr_SRS_PURESOUND_RESERVED1+8
            #define SRS_PURESOUND_AeqIir3Coefs_addr              DSP2XboxAddr_SRS_PURESOUND_RESERVED1+13
            #define SRS_PURESOUND_AeqIir4Coefs_addr              DSP2XboxAddr_SRS_PURESOUND_RESERVED1+18
            #define SRS_PURESOUND_AeqIir5Coefs_addr              DSP2XboxAddr_SRS_PURESOUND_RESERVED1+23
            #define SRS_PURESOUND_AeqIir6Coefs_addr              DSP2XboxAddr_SRS_PURESOUND_RESERVED1+28
            #define SRS_PURESOUND_AeqIir7Coefs_addr              DSP2XboxAddr_SRS_PURESOUND_RESERVED1+33
            #define SRS_PURESOUND_AeqIir8Coefs_addr              DSP2XboxAddr_SRS_PURESOUND_RESERVED1+38
            #define SRS_PURESOUND_AeqIir1_iwl_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED1+43
            #define SRS_PURESOUND_AeqIir2_iwl_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED1+44
            #define SRS_PURESOUND_AeqIir3_iwl_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED1+45
            #define SRS_PURESOUND_AeqIir4_iwl_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED1+46
            #define SRS_PURESOUND_AeqIir5_iwl_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED1+47
            #define SRS_PURESOUND_AeqIir6_iwl_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED1+48
            #define SRS_PURESOUND_AeqIir7_iwl_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED1+49
            #define SRS_PURESOUND_AeqIir8_iwl_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED1+50
            #define SRS_PURESOUND_AeqIir_Gain_iwl_addr           DSP2XboxAddr_SRS_PURESOUND_RESERVED1+51
            #define SRS_PURESOUND_AeqIir_Gain_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED1+52
            #define SRS_PURESOUND_AeqFir_iwl_addr                DSP2XboxAddr_SRS_PURESOUND_RESERVED1+53
            #define SRS_PURESOUND_AeqFir_Gain_iwl_addr           DSP2XboxAddr_SRS_PURESOUND_RESERVED1+54
            #define SRS_PURESOUND_AeqFir_Gain_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED1+55
            #define SRS_PURESOUND_AeqFirCoefs_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED1+64

            #define DSP2XboxAddr_SRS_PURESOUND_RESERVED2         SRS_PURESOUND_AeqFirCoefs_addr+1
            #define SRS_PURESOUND_mDummy_addr                    DSP2XboxAddr_SRS_PURESOUND_RESERVED2
            #define SRS_PURESOUND_SRS_EN_BITS_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED2+1
            #define SRS_PURESOUND_mInputGain_addr                DSP2XboxAddr_SRS_PURESOUND_RESERVED2+2
            #define SRS_PURESOUND_mOutputGain_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED2+3
            #define SRS_PURESOUND_mBypassGain_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED2+4
            #define SRS_PURESOUND_mHPFfc_addr                    DSP2XboxAddr_SRS_PURESOUND_RESERVED2+5
            #define SRS_PURESOUND_hlInputGain_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED2+25
            #define SRS_PURESOUND_hlOutputGain_addr              DSP2XboxAddr_SRS_PURESOUND_RESERVED2+26
            #define SRS_PURESOUND_hlBypassGain_addr              DSP2XboxAddr_SRS_PURESOUND_RESERVED2+27
            #define SRS_PURESOUND_hlLimiterboost_addr            DSP2XboxAddr_SRS_PURESOUND_RESERVED2+28
            #define SRS_PURESOUND_hlHardLimit_addr               DSP2XboxAddr_SRS_PURESOUND_RESERVED2+29
            #define SRS_PURESOUND_hlDelaylen_addr                DSP2XboxAddr_SRS_PURESOUND_RESERVED2+30
            #define SRS_PURESOUND_AeqInputGain_addr              DSP2XboxAddr_SRS_PURESOUND_RESERVED2+32
            #define SRS_PURESOUND_AeqOutputGain_addr             DSP2XboxAddr_SRS_PURESOUND_RESERVED2+33
            #define SRS_PURESOUND_AeqBypassGain_addr             DSP2XboxAddr_SRS_PURESOUND_RESERVED2+34

            #define DSP2XboxAddr_SRS_PURESOUND_RESERVED_END      SRS_PURESOUND_AeqBypassGain_addr

        /* atv */
        //reserved XBox for ATV
             #define DSP2XboxAddr_ATV_RESERVED1                  DSP2XboxAddr_SRS_PURESOUND_RESERVED_END+1
             #define DSP2XboxAddr_AU_PAL_SYS_THRESHOLD           DSP2XboxAddr_ATV_RESERVED1
             #define DSP2XboxAddr_SIF_PM_GAIN_TBL_PAL            DSP2XboxAddr_AU_PAL_SYS_THRESHOLD+12
             #define DSP2XboxAddr_SIF_PM_GAIN_TBL_BTSC           DSP2XboxAddr_AU_PAL_SYS_THRESHOLD+12
             #define DSP2XboxAddr_ATV_RESERVED_END               DSP2XboxAddr_SIF_PM_GAIN_TBL_BTSC+8-1

    /************************************************
    *   For Compile Pass  mailbox (Need to remove later)
    ************************************************/

        #define D2M_MBOX_INTR_CMDTYPE                MB_2DB2
        #define DSP1PmAddr_ipSecurity                0x0FF2
        #define D2M_MBOX_ENC_LINEADDR                MB_2DAC                 //MPEG Encoder
        #define D2M_MBOX_ENC_LINESIZE                MB_2DAE                 //MPEG Encoder
        #define M2D_MBOX_PIO_ID                      MB_2D8A
        #define DSP1PmAddr_smpRate              0x0FF7
        #define D2M_MBOX_HDMI_NPCM_LOCK         MB_2DB4     //[ 3:0] Always in LSB Nibble
        #define MBOX_DBGCMD_RELOAD_DTV1_BEG         0xF000
        #define MBOX_DBGCMD_RELOAD_DTV1_END         0xF100
        #define MBOX_DBGCMD_RELOAD_DTV2_BEG         0xF600
        #define MBOX_DBGCMD_RELOAD_DTV2_END         0xF700
        #define DSP1DmAddr_sys_IoInfo                          NULL
        #define M2D_MBOX_MM_FILEIN_TAG          MB_2D8C
        #define MBOX_DSP_INIT_ACK                   0xE300
        #define D2M_MBOX_DBG_RESULT2            MB_2DBE
        #define D2M_MBOX_DBG_RESULT1            MB_2DBC
        #define M2D_MBOX_DEC_CTRL                  MB_2D86
        #define M2D_MBOX_DBG_CMD1               MB_2D9C

         #define D2M_MBOX_DBG_RESULT1            MB_2DBC
         #define MBOX_DSP_INIT_ACK                   0xE300
         #define D2M_MBOX_DBG_RESULT2            MB_2DBE
         #define MBOX_DSP_RELOAD_ACK1                0x0033
         #define MBOX_DSP_RELOAD_ACK2                0x0077
         #define M2D_MBOX_DEC_CTRL               MB_2D86
         #define D2M_MBOX_SAMPLERATE             MB_2DA6
         #define DSP1DmAddr_dec1_param           0x47A0
         #define DSP1DmAddr_dec1_info            0x47C0
         #define DSP1DmAddr_dec1_omx_param       0x42B4
         #define M2D_MBOX_UNI_PCM3_WRPTR         MB_2D94
         #define DSP1PmAddr_video_TD             0x0FF1
         #define D2M_MBOX_UNI_PCM_BUFFEBT        MB_2D6A

    //// SIF    /* SIF DSP PM vars */        //  for SIF PAL DSP PM vars //
        #define ADDR_gain_base		    0x1921
        #define ADDR_thr_base		     0x1A20
        #define ADDR_pfir_base              0x1A90        //  for SIF BTSC DSP PM vars //
        #define BTSC_COMPILE_OPTION_Addr          0x19F1   // len 1
        #define BTSC_OUTPUT_GAIN_Addr               0x1A21     // len 2
        #define BTSC_THRESHOLD_Addr                    0x1A23       // len 10
        #define MTS_OUTPUT_GAIN_Addr                 0x1A34   //len 6
        #define SIF_AGC_THRESHOLD_Addr               0x192D   //len 3            /// PAL gain setting address
        #define ADDR_fm_stdM_gain          ADDR_gain_base           // len = 4
        #define ADDR_fm_stdX_gain          ADDR_fm_stdM_gain+4  // len = 4
        #define ADDR_nicam_gain             ADDR_fm_stdX_gain+4   // len = 2
        #define ADDR_am_gain                  ADDR_nicam_gain+2        // len = 2
        #define ADDR_agc_gain                 ADDR_am_gain+2            // len = 24            // PAL threshold setting address
        #define ADDR_a2_stdM_thr             ADDR_thr_base               				 // len = 15
        #define ADDR_a2_stdBG_thr           ADDR_a2_stdM_thr+15    				 // len = 15
        #define ADDR_a2_stdDK_thr           ADDR_a2_stdBG_thr+15 				 // len = 15
        #define ADDR_a2_stdI_thr              ADDR_a2_stdDK_thr+15   				 // len = 4
        #define ADDR_am_thr                     ADDR_a2_stdI_thr+4        				 // len = 3
        #define ADDR_hidev_stdM_thr        ADDR_am_thr+3            				 // len = 4
        #define ADDR_hidev_stdBG_thr      ADDR_hidev_stdM_thr+4  				 // len = 4
        #define ADDR_hidev_stdDK_thr      ADDR_hidev_stdBG_thr+4  				 // len = 4
        #define ADDR_hidev_stdI_thr         ADDR_hidev_stdDK_thr+4 				 // len = 4
        #define ADDR_nicam_stdBG_pherr_thr        ADDR_hidev_stdI_thr+4  	        //len = 3
        #define ADDR_nicam_stdI_pherr_thr           ADDR_nicam_stdBG_pherr_thr+3  // len = 3
        #define ADDR_a2_bg_nicam_fm_nsr_thr     0x186F	 // len = 1
        #define ADDR_a2_dk_nicam_fm_nsr_thr     0x1870 	// len = 1            // pfir setting address
        #define ADDR_hidev_demfir          ADDR_pfir_base                 // len = 15
        #define ADDR_fm_ch1_pfir           ADDR_hidev_demfir+16       // len = 30
        #define ADDR_fm_ch2_pfir           ADDR_fm_ch1_pfir+30         // len = 30
        #define ADDR_hidev_lv1_pfir        ADDR_fm_ch2_pfir+30        // len = 20
        #define ADDR_hidev_lv2_pfir        ADDR_hidev_lv1_pfir+20     // len = 20
        #define ADDR_hidev_lv3_pfir        ADDR_hidev_lv2_pfir+20     // len = 20            // BTSC threshold setting address
        #define HIDEV_NSR_THRESHOLD_Addr            BTSC_THRESHOLD_Addr+10   // len 2
        #define BTSC_MONO_AMP_THRESHOLD_Addr    HIDEV_NSR_THRESHOLD_Addr+2   // len 2
        #define HIDEV_AMP_THRESHOLD_Addr    BTSC_MONO_AMP_THRESHOLD_Addr+2  // len 2
        #define BTSC_SAP_PHASE_DIFF_CLIP_THR_Addr    HIDEV_AMP_THRESHOLD_Addr+2   // len 1
        #define BTSC_PILOT_DEBOUNCE_THRESHOLD_Addr   MTS_OUTPUT_GAIN_Addr+6   // len 3
        #define BTSC_SAP_ON_DETECTION_DEBOUNCE_THRESHOLD_Addr BTSC_PILOT_DEBOUNCE_THRESHOLD_Addr+3 // len 1

        #define M2S_MBOX_MM_FILEIN_TAG              MB_2DCC             //[7:0]
        #define DSP2PmAddr_smpRate                       0x0D49
        #define DSP2PmAddr_soundMode                   0x0D4A
        #define DSP2DmAddr_dec1_param                 0x4390
        #define DSP2DmAddr_hdmi_debugInfo                   0x396A
        #define DSP2DmAddr_spdif_debugInfo                  0x3832

    /* SIF DSP PM vars */
        /*  for SIF PAL DSP PM vars */

            #define ADDR_gain_base_2                  NULL
            #define ADDR_thr_base_2                   NULL
            #define ADDR_pfir_base_2                  NULL
            //  for SIF BTSC DSP PM vars //
            #define BTSC_COMPILE_OPTION_Addr_2        NULL   // len 1
            #define BTSC_OUTPUT_GAIN_Addr_2           NULL   // len 2
            #define BTSC_THRESHOLD_Addr_2             NULL   // len 10
            #define MTS_OUTPUT_GAIN_Addr_2            NULL   //len 6
            #define SIF_AGC_THRESHOLD_Addr_2          NULL   //len 3

            /// PAL gain setting address
            #define ADDR_fm_stdM_gain_2               NULL           // len = 4
            #define ADDR_fm_stdX_gain_2               NULL     // len = 4
            #define ADDR_nicam_gain_2                 NULL      // len = 2
            #define ADDR_am_gain_2                    NULL        // len = 2
            #define ADDR_agc_gain_2                   NULL          // len = 24

            // PAL threshold setting address
            #define ADDR_a2_stdM_thr_2                NULL            // len = 15
            #define ADDR_a2_stdBG_thr_2               NULL      // len = 15
            #define ADDR_a2_stdDK_thr_2               NULL         // len = 15
            #define ADDR_a2_stdI_thr_2                NULL     // len = 4
            #define ADDR_am_thr_2                     NULL       // len = 3
            #define ADDR_hidev_stdM_thr_2             NULL           // len = 4
            #define ADDR_hidev_stdBG_thr_2            NULL    // len = 4
            #define ADDR_hidev_stdDK_thr_2            NULL  // len = 4
            #define ADDR_hidev_stdI_thr_2             NULL   // len = 4
            #define ADDR_nicam_stdBG_pherr_thr_2      NULL    //len = 3
            #define ADDR_nicam_stdI_pherr_thr_2       NULL  // len = 3
            #define ADDR_a2_bg_nicam_fm_nsr_thr_2     NULL     // len = 1
            #define ADDR_a2_dk_nicam_fm_nsr_thr_2     NULL     // len = 1

            // pfir setting address
            #define ADDR_hidev_demfir_2               NULL             // len = 15
            #define ADDR_fm_ch1_pfir_2               NULL       // len = 30
            #define ADDR_fm_ch2_pfir_2                NULL       // len = 30
            #define ADDR_hidev_lv1_pfir_2             NULL        // len = 20
            #define ADDR_hidev_lv2_pfir_2             NULL     // len = 20
            #define ADDR_hidev_lv3_pfir_2             NULL     // len = 20

            // BTSC threshold setting address
            #define HIDEV_NSR_THRESHOLD_Addr_2        NULL           // len 2
            #define BTSC_MONO_AMP_THRESHOLD_Addr_2    NULL        // len 2
            #define HIDEV_AMP_THRESHOLD_Addr_2        NULL    // len 2

            #define BTSC_SAP_PHASE_DIFF_CLIP_THR_Addr_2    NULL   // len 1
            #define BTSC_PILOT_DEBOUNCE_THRESHOLD_Addr_2   NULL       // len 3
            #define BTSC_SAP_ON_DETECTION_DEBOUNCE_THRESHOLD_Addr_2 NULL // len 1

    /************************************************
    *   End for  Compile Pass  mailbox (Need to remove later)
    ************************************************/


#endif  //_AUDIO_COMM2_H_

