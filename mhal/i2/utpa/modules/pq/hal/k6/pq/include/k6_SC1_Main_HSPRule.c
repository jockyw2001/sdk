
U8 MST_HSPRule_IP_Index_SC1_Main[PQ_HSPRule_IP_NUM_SC1_Main]=
{
    PQ_IP_HSP_Y_SC1_Main,
    PQ_IP_HSP_C_SC1_Main,
    PQ_IP_SRAM2_SC1_Main,
};


U8 MST_HSPRule_Array_SC1_Main[PQ_HSPRule_NUM_SC1_Main][PQ_HSPRule_IP_NUM_SC1_Main]=
{
    {//PreV_ScalingDown_Interlace, 0
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_NULL,
    },
    {//PreV_ScalingDown_Progressive, 1
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_NULL,
    },
    {//ScalingDown_00x_YUV, 2
        PQ_IP_HSP_Y_SRAM_2_4Tap_SC1_Main, PQ_IP_HSP_C_C_SRAM_1_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_00x_RGB, 3
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_01x_YUV, 4
        PQ_IP_HSP_Y_SRAM_2_4Tap_SC1_Main, PQ_IP_HSP_C_C_SRAM_1_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_01x_RGB, 5
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_02x_YUV, 6
        PQ_IP_HSP_Y_SRAM_2_4Tap_SC1_Main, PQ_IP_HSP_C_C_SRAM_1_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_02x_RGB, 7
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_03x_YUV, 8
        PQ_IP_HSP_Y_SRAM_2_4Tap_SC1_Main, PQ_IP_HSP_C_C_SRAM_1_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_03x_RGB, 9
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_04x_YUV, 10
        PQ_IP_HSP_Y_SRAM_2_4Tap_SC1_Main, PQ_IP_HSP_C_C_SRAM_1_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_04x_RGB, 11
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_05x_YUV, 12
        PQ_IP_HSP_Y_SRAM_2_4Tap_SC1_Main, PQ_IP_HSP_C_C_SRAM_1_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_05x_RGB, 13
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_06x_YUV, 14
        PQ_IP_HSP_Y_SRAM_2_4Tap_SC1_Main, PQ_IP_HSP_C_C_SRAM_1_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_06x_RGB, 15
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_07x_YUV, 16
        PQ_IP_HSP_Y_SRAM_2_4Tap_SC1_Main, PQ_IP_HSP_C_C_SRAM_1_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_07x_RGB, 17
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_08x_YUV, 18
        PQ_IP_HSP_Y_SRAM_2_4Tap_SC1_Main, PQ_IP_HSP_C_C_SRAM_1_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_08x_RGB, 19
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_09x_YUV, 20
        PQ_IP_HSP_Y_SRAM_2_4Tap_SC1_Main, PQ_IP_HSP_C_C_SRAM_1_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_09x_RGB, 21
        PQ_IP_HSP_Y_Bilinear_SC1_Main, PQ_IP_HSP_C_Bilinear_SC1_Main, PQ_IP_SRAM2_InvSinc4Tc4p4Fc50Apass01Astop55_SC1_Main,
    },
    {//ScalingDown_10x_YUV, 22
        PQ_IP_NULL, PQ_IP_NULL, PQ_IP_NULL,
    },
    {//ScalingDown_10x_RGB, 23
        PQ_IP_NULL, PQ_IP_NULL, PQ_IP_NULL,
    },
};
