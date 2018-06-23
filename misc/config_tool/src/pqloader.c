#include "mi_common.h"
#include "mi_syscfg_datatype.h"
#include "mapi_utility.h"
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define ASSERT(e) if(!(e)){printf("assert at %s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);printf("\n");*(int*)0 = 0;}
#define error_wrapper(e) if(e == -1){ printf("%s:%d fail(%s)\n", __FUNCTION__, __LINE__, strerror(errno)); return;}

void LoadPQConfig(void)
{
    MI_BOOL b_Ret = TRUE;

    dictionary *config = mapi_config_helper("/config/PQ_general/PQConfig.ini");
    //MI_U8* tempArray =NULL;
    //int size;

    MI_SYSCFG_PQConfig_t pqconfig;
    printf("************************PQ Config Start*****************************\n");
    //reading numeric value
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_MAX_VIDEO_DBC"         ,&pqconfig.m_u8DBC_MAX_VIDEO_DBC          );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_MID_VIDEO_DBC"         ,&pqconfig.m_u8DBC_MID_VIDEO_DBC          );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_MIN_VIDEO_DBC"         ,&pqconfig.m_u8DBC_MIN_VIDEO_DBC          );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_MAX_PWM"               ,&pqconfig.m_u8DBC_MAX_PWM                );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_BACKLIGHT_THRES"       ,&pqconfig.m_u8DBC_BACKLIGHT_THRES        );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_MIN_PWM"               ,&pqconfig.m_u8DBC_MIN_PWM                );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_Y_GAIN_M"              ,&pqconfig.m_u8DBC_Y_GAIN_M               );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_Y_GAIN_L"              ,&pqconfig.m_u8DBC_Y_GAIN_L               );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_C_GAIN_M"              ,&pqconfig.m_u8DBC_C_GAIN_M               );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_C_GAIN_L"              ,&pqconfig.m_u8DBC_C_GAIN_L               );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_ALPHA_BLENDING_CURRENT",&pqconfig.m_u8DBC_ALPHA_BLENDING_CURRENT );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_AVG_DELTA"             ,&pqconfig.m_u8DBC_AVG_DELTA              );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_FAST_ALPHABLENDING"    ,&pqconfig.m_u8DBC_FAST_ALPHABLENDING     );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_LOOP_DLY_H"            ,&pqconfig.m_u8DBC_LOOP_DLY_H             );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_LOOP_DLY_MH"           ,&pqconfig.m_u8DBC_LOOP_DLY_MH            );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_LOOP_DLY_ML"           ,&pqconfig.m_u8DBC_LOOP_DLY_ML            );
    b_Ret &= mapi_config_helper_GetU8(config, "DBC:g_DBC_LOOP_DLY_L"            ,&pqconfig.m_u8DBC_LOOP_DLY_L            );
    printf("DBC Value=\n\t%d,%d,%d\n\t%d,%d,%d\n\t%d,%d,%d\n",pqconfig.m_u8DBC_MAX_VIDEO_DBC,pqconfig.m_u8DBC_MID_VIDEO_DBC,pqconfig.m_u8DBC_MIN_VIDEO_DBC,pqconfig.m_u8DBC_MAX_PWM,pqconfig.m_u8DBC_BACKLIGHT_THRES,pqconfig.m_u8DBC_MIN_PWM,pqconfig.m_u8DBC_Y_GAIN_M,pqconfig.m_u8DBC_Y_GAIN_L,pqconfig.m_u8DBC_C_GAIN_M);

    /* ******************************************************
     *                                                      *
     * Reading tDynamicNRTbl_Motion from section DNRTable   *
     *                                                      *
     * ******************************************************/
    // read the cols and rows of array
    if( (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_Motion_rows"           ,&pqconfig.m_u8tDynamicNRTbl_Motion_rows            ) ) &&
            (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_Motion_cols"           ,&pqconfig.m_u8tDynamicNRTbl_Motion_cols            ) ) )
    {
        printf("ROWS =%d, COLS=%d\n",pqconfig.m_u8tDynamicNRTbl_Motion_rows,pqconfig.m_u8tDynamicNRTbl_Motion_cols);
            // cols number must equal to DYNAMIC_NR_TBL_REG_NUM value
            if(DYNAMIC_NR_TBL_REG_NUM != pqconfig.m_u8tDynamicNRTbl_Motion_cols)
            {
                ASSERT(0);
            }

        MI_U8 *m_ppu8tDynamicNRTbl_Motion[pqconfig.m_u8tDynamicNRTbl_Motion_rows];
        // malloc array to lower level, for reading array from ini files
        pqconfig.m_pu8tDynamicNRTbl_Motion = (MI_U8*)calloc(pqconfig.m_u8tDynamicNRTbl_Motion_rows,pqconfig.m_u8tDynamicNRTbl_Motion_cols);
        if(NULL== pqconfig.m_pu8tDynamicNRTbl_Motion )
            ASSERT(0);
        int i;
        for(i=0;i<pqconfig.m_u8tDynamicNRTbl_Motion_rows;i++)
        {
            m_ppu8tDynamicNRTbl_Motion[i] = pqconfig.m_pu8tDynamicNRTbl_Motion + i*pqconfig.m_u8tDynamicNRTbl_Motion_cols;
        }
        // reading array from ini files
        b_Ret &= mapi_config_helper_Get2DArray(config, "DNRTable:tDynamicNRTbl_Motion",m_ppu8tDynamicNRTbl_Motion,pqconfig.m_u8tDynamicNRTbl_Motion_rows,pqconfig.m_u8tDynamicNRTbl_Motion_cols);
    }
    else
    {
        b_Ret = FALSE;
    }


    /* ***************************************************************
     *                                                               *
     * Reading tDynamicNRTbl_Motion_HDMI_DTV from section DNRTable   *
     *                                                               *
     * ***************************************************************/
    if( (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_Motion_HDMI_DTV_rows"           ,&pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows            )) &&
            (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_Motion_HDMI_DTV_cols"           ,&pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols            )))
    {
        // cols number must equal to DYNAMIC_NR_TBL_REG_NUM value
        if(DYNAMIC_NR_TBL_REG_NUM != pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols)
        {
            ASSERT(0);
        }
        MI_U8 *m_ppu8tDynamicNRTbl_Motion_HDMI_DTV[pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows];
        pqconfig.m_pu8tDynamicNRTbl_Motion_HDMI_DTV = (MI_U8*)calloc(pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows,pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols);
        if(NULL== pqconfig.m_pu8tDynamicNRTbl_Motion_HDMI_DTV )
            ASSERT(0);
        int i;
        for(i=0;i<pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows;i++)
        {
            m_ppu8tDynamicNRTbl_Motion_HDMI_DTV[i] = (MI_U8*)pqconfig.m_pu8tDynamicNRTbl_Motion_HDMI_DTV + i*pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols;
        }
        b_Ret &= mapi_config_helper_Get2DArray(config, "DNRTable:tDynamicNRTbl_Motion_HDMI_DTV",m_ppu8tDynamicNRTbl_Motion_HDMI_DTV,pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows,pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols);
    }
    else
    {
        b_Ret = FALSE;
    }


    /* ***************************************************************
     *                                                               *
     * Reading tDynamicNRTbl_Motion_COMP_PC from section DNRTable    *
     *                                                               *
     * ***************************************************************/
    if( (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_Motion_COMP_PC_rows"           ,&pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_rows            )) &&
            (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_Motion_COMP_PC_cols"           ,&pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_cols            )))
    {
        // cols number must equal to DYNAMIC_NR_TBL_REG_NUM value
        if(DYNAMIC_NR_TBL_REG_NUM != pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_cols)
        {
            ASSERT(0);
        }
        MI_U8 *m_ppu8tDynamicNRTbl_Motion_COMP_PC[pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_rows];
        pqconfig.m_pu8tDynamicNRTbl_Motion_COMP_PC = (MI_U8*)calloc(pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_rows,pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_cols);
        if(NULL== pqconfig.m_pu8tDynamicNRTbl_Motion_COMP_PC )
            ASSERT(0);
        int i;
        for(i=0;i<pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_rows;i++)
        {
            m_ppu8tDynamicNRTbl_Motion_COMP_PC[i] = (MI_U8*)pqconfig.m_pu8tDynamicNRTbl_Motion_COMP_PC + i*pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_cols;
        }
        b_Ret &= mapi_config_helper_Get2DArray(config, "DNRTable:tDynamicNRTbl_Motion_COMP_PC",m_ppu8tDynamicNRTbl_Motion_COMP_PC,pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_rows,pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_cols);
    }
    else
    {
        b_Ret = FALSE;
    }


    /* ***************************************************************
     *                                                               *
     * Reading tDynamicNRTbl_MISC_Param from section DNRTable        *
     *                                                               *
     * ***************************************************************/
    if( (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_MISC_Param_rows"           ,&pqconfig.m_u8tDynamicNRTbl_MISC_Param_rows            )) &&
            (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_MISC_Param_cols"           ,&pqconfig.m_u8tDynamicNRTbl_MISC_Param_cols            )))
    {
        MI_U8 *m_ppu8tDynamicNRTbl_MISC_Param[pqconfig.m_u8tDynamicNRTbl_MISC_Param_rows];
        pqconfig.tDynamicNRTbl_MISC_Param = (MAPI_PQL_DYNAMIC_NR_MISC_PARAM*)calloc(pqconfig.m_u8tDynamicNRTbl_MISC_Param_rows,sizeof(MAPI_PQL_DYNAMIC_NR_MISC_PARAM));
        int i;
        for(i=0;i<pqconfig.m_u8tDynamicNRTbl_MISC_Param_rows;i++)
        {
            m_ppu8tDynamicNRTbl_MISC_Param[i] = (MI_U8*)(pqconfig.tDynamicNRTbl_MISC_Param+i);
        }
        b_Ret &= mapi_config_helper_Get2DArray(config, "DNRTable:tDynamicNRTbl_MISC_Param",m_ppu8tDynamicNRTbl_MISC_Param,pqconfig.m_u8tDynamicNRTbl_MISC_Param_rows,pqconfig.m_u8tDynamicNRTbl_MISC_Param_cols);
    }
    else
    {
        b_Ret = FALSE;
    }


    /* ***************************************************************
     *                                                               *
     * Reading tDynamicNRTbl_MISC_Luma from section DNRTable        *
     *                                                               *
     * ***************************************************************/
    if( (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_MISC_Luma_rows"           ,&pqconfig.m_u8tDynamicNRTbl_MISC_Luma_rows            )) &&
            (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_MISC_Luma_cols"           ,&pqconfig.m_u8tDynamicNRTbl_MISC_Luma_cols            )))
    {
        // rows number must equal to DYNAMIC_NR_TBL_REG_NUM value
        if(DYNAMIC_NR_TBL_REG_NUM != pqconfig.m_u8tDynamicNRTbl_MISC_Luma_rows)
        {
            printf(" rows number must equal to DYNAMIC_NR_TBL_REG_NUM value\n");
            ASSERT(0);
        }
        MI_U8 *m_ppu8tDynamicNRTbl_MISC_Luma[pqconfig.m_u8tDynamicNRTbl_MISC_Luma_rows];
        pqconfig.tDynamicNRTbl_MISC_Luma = (MAPI_PQL_DYNAMIC_NR_MISC_PARAM_LUMA*)calloc(pqconfig.m_u8tDynamicNRTbl_MISC_Luma_rows,sizeof(MAPI_PQL_DYNAMIC_NR_MISC_PARAM_LUMA));
        int i;
        for(i=0;i<pqconfig.m_u8tDynamicNRTbl_MISC_Luma_rows;i++)
        {
            m_ppu8tDynamicNRTbl_MISC_Luma[i] = (MI_U8*)(pqconfig.tDynamicNRTbl_MISC_Luma+i);
        }
        b_Ret &= mapi_config_helper_Get2DArray(config, "DNRTable:tDynamicNRTbl_MISC_Luma",m_ppu8tDynamicNRTbl_MISC_Luma,pqconfig.m_u8tDynamicNRTbl_MISC_Luma_rows,pqconfig.m_u8tDynamicNRTbl_MISC_Luma_cols);
    }
    else
    {
        b_Ret = FALSE;
    }



    /* ***************************************************************
     *                                                               *
     * Reading tDynamicNRTbl_Noise from section DNRTable             *
     *                                                               *
     * ***************************************************************/
    if( (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_Noise_rows"           ,&pqconfig.m_u8tDynamicNRTbl_Noise_rows            )) &&
            (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNRTbl_Noise_cols"           ,&pqconfig.m_u8tDynamicNRTbl_Noise_cols            )))
    {
        // cols number must equal to DYNAMIC_NR_TBL_REG_NUM value
        if(DYNAMIC_NR_TBL_REG_NUM != pqconfig.m_u8tDynamicNRTbl_Noise_cols)
        {

            printf(" cols number must equal to DYNAMIC_NR_TBL_REG_NUM value\n");
            ASSERT(0);
        }
        MI_U8 *m_ppu8tDynamicNRTbl_Noise[pqconfig.m_u8tDynamicNRTbl_Noise_rows];
        pqconfig.m_pu8tDynamicNRTbl_Noise = (MI_U8*)calloc(pqconfig.m_u8tDynamicNRTbl_Noise_rows,pqconfig.m_u8tDynamicNRTbl_Noise_cols);
        if(NULL== pqconfig.m_pu8tDynamicNRTbl_Noise )
            ASSERT(0);
        int i;
        for(i=0;i<pqconfig.m_u8tDynamicNRTbl_Noise_rows;i++)
        {
            m_ppu8tDynamicNRTbl_Noise[i] = (MI_U8*)pqconfig.m_pu8tDynamicNRTbl_Noise + i*pqconfig.m_u8tDynamicNRTbl_Noise_cols;
        }
        b_Ret &= mapi_config_helper_Get2DArray(config, "DNRTable:tDynamicNRTbl_Noise",m_ppu8tDynamicNRTbl_Noise,pqconfig.m_u8tDynamicNRTbl_Noise_rows,pqconfig.m_u8tDynamicNRTbl_Noise_cols);
    }
    else
    {
        b_Ret = FALSE;
    }



    /* ***************************************************************
     *                                                               *
     * Reading tDynamicNR_MISC_Noise from section DNRTable             *
     *                                                               *
     * ***************************************************************/
    if( (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNR_MISC_Noise_rows"           ,&pqconfig.m_u8tDynamicNR_MISC_Noise_rows            ))&&
            (TRUE == mapi_config_helper_GetU8(config, "DNRTable:tDynamicNR_MISC_Noise_cols"           ,&pqconfig.m_u8tDynamicNR_MISC_Noise_cols            )))
    {
        MI_U8 *m_ppu8tDynamicNR_MISC_Noise[pqconfig.m_u8tDynamicNR_MISC_Noise_rows];
        pqconfig.tDynamicNR_MISC_Noise = (MAPI_PQL_DYNAMIC_NR_MISC_PARAM_NOISE*)calloc(pqconfig.m_u8tDynamicNR_MISC_Noise_rows,sizeof(MAPI_PQL_DYNAMIC_NR_MISC_PARAM_NOISE));
        int i;
        for(i=0;i<pqconfig.m_u8tDynamicNR_MISC_Noise_rows;i++)
        {
            m_ppu8tDynamicNR_MISC_Noise[i] = (MI_U8*)(pqconfig.tDynamicNR_MISC_Noise+i);
        }
        b_Ret &= mapi_config_helper_Get2DArray(config, "DNRTable:tDynamicNR_MISC_Noise",m_ppu8tDynamicNR_MISC_Noise,pqconfig.m_u8tDynamicNR_MISC_Noise_rows,pqconfig.m_u8tDynamicNR_MISC_Noise_cols);
    }
    else
    {
        b_Ret = FALSE;
    }

    printf("************************PQ Config End*****************************\n");
    int fd = open("/proc/mi_modules/common/pq_info", O_WRONLY);
    error_wrapper(fd);
    error_wrapper(write(fd, &pqconfig, offsetof(MI_SYSCFG_PQConfig_t, m_pu8tDynamicNRTbl_Noise)));
    close(fd);
    fd = open("/proc/mi_modules/common/noise_table", O_WRONLY);
    error_wrapper(fd);
    error_wrapper(write(fd, pqconfig.m_pu8tDynamicNRTbl_Noise, pqconfig.m_u8tDynamicNRTbl_Noise_rows*pqconfig.m_u8tDynamicNRTbl_Noise_cols));
    close(fd);
    fd = open("/proc/mi_modules/common/motion_table", O_WRONLY);
    error_wrapper(fd);
    error_wrapper(write(fd, pqconfig.m_pu8tDynamicNRTbl_Motion, pqconfig.m_u8tDynamicNRTbl_Motion_rows*pqconfig.m_u8tDynamicNRTbl_Motion_cols));
    close(fd);
    fd = open("/proc/mi_modules/common/motion_hdmi_dtv_table", O_WRONLY);
    error_wrapper(fd);
    error_wrapper(write(fd, pqconfig.m_pu8tDynamicNRTbl_Motion_HDMI_DTV, pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows*pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols));
    close(fd);
    fd = open("/proc/mi_modules/common/motion_comp_pc_table", O_WRONLY);
    error_wrapper(fd);
    error_wrapper(write(fd, pqconfig.m_pu8tDynamicNRTbl_Motion_COMP_PC, pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_rows*pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_cols));
    close(fd);
    fd = open("/proc/mi_modules/common/misc_table", O_WRONLY);
    error_wrapper(fd);
    error_wrapper(write(fd, pqconfig.tDynamicNR_MISC_Noise, pqconfig.m_u8tDynamicNR_MISC_Noise_rows*pqconfig.m_u8tDynamicNR_MISC_Noise_cols));
    close(fd);
    fd = open("/proc/mi_modules/common/misc_luma_table", O_WRONLY);
    error_wrapper(fd);
    error_wrapper(write(fd, pqconfig.tDynamicNRTbl_MISC_Luma, pqconfig.m_u8tDynamicNRTbl_MISC_Luma_rows*pqconfig.m_u8tDynamicNRTbl_MISC_Luma_cols));
    close(fd);
    fd = open("/proc/mi_modules/common/misc_param_table", O_WRONLY);
    error_wrapper(fd);
    error_wrapper(write(fd, pqconfig.tDynamicNRTbl_MISC_Param, pqconfig.m_u8tDynamicNRTbl_MISC_Param_rows*pqconfig.m_u8tDynamicNRTbl_MISC_Param_cols));
    close(fd);
}

void dump_table(const char *tt, MI_U8 *v, int r, int c){
    printf("start dump [%s](%d, %d)\n", tt, r, c);
    int i, j;
    for(i = 0; i < r; ++i){
        printf("{");
        for(j = 0; j < c; ++j){
            printf("0x%02x,", v[i*c+j]);
        }
        printf("}\n");
    }
    printf("end dump\n");
}


void DumpPQConfig(void){
    MI_SYSCFG_PQConfig_t pqconfig = {0};
    int fd = open("/proc/mi_modules/common/pq_info", O_RDONLY);
    error_wrapper(fd);
    error_wrapper(read(fd, &pqconfig, offsetof(MI_SYSCFG_PQConfig_t, m_pu8tDynamicNRTbl_Noise)));
    close(fd);
    printf("DBC Value=\n\t%d,%d,%d\n\t%d,%d,%d\n\t%d,%d,%d\n",pqconfig.m_u8DBC_MAX_VIDEO_DBC,pqconfig.m_u8DBC_MID_VIDEO_DBC,pqconfig.m_u8DBC_MIN_VIDEO_DBC,pqconfig.m_u8DBC_MAX_PWM,pqconfig.m_u8DBC_BACKLIGHT_THRES,pqconfig.m_u8DBC_MIN_PWM,pqconfig.m_u8DBC_Y_GAIN_M,pqconfig.m_u8DBC_Y_GAIN_L,pqconfig.m_u8DBC_C_GAIN_M);
    fd = open("/proc/mi_modules/common/motion_table", O_RDONLY);
    error_wrapper(fd);
    MI_U8 m_pu8tDynamicNRTbl_Motion[pqconfig.m_u8tDynamicNRTbl_Motion_rows*pqconfig.m_u8tDynamicNRTbl_Motion_cols];
    error_wrapper(read(fd, m_pu8tDynamicNRTbl_Motion, pqconfig.m_u8tDynamicNRTbl_Motion_rows*pqconfig.m_u8tDynamicNRTbl_Motion_cols));
    close(fd);
    dump_table("motion_table", m_pu8tDynamicNRTbl_Motion, pqconfig.m_u8tDynamicNRTbl_Motion_rows, pqconfig.m_u8tDynamicNRTbl_Motion_cols);
    fd = open("/proc/mi_modules/common/motion_hdmi_dtv_table", O_RDONLY);
    error_wrapper(fd);
    MI_U8 m_pu8tDynamicNRTbl_Motion_HDMI_DTV[pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows*pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols];
    error_wrapper(read(fd, m_pu8tDynamicNRTbl_Motion_HDMI_DTV, pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows*pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols));
    close(fd);
    dump_table("motion_hdmi_dtv_table", m_pu8tDynamicNRTbl_Motion_HDMI_DTV, pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows, pqconfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols);
    fd = open("/proc/mi_modules/common/motion_comp_pc_table", O_RDONLY);
    error_wrapper(fd);
    MI_U8 m_pu8tDynamicNRTbl_Motion_COMP_PC[pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_rows*pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_cols];
    error_wrapper(read(fd, m_pu8tDynamicNRTbl_Motion_COMP_PC, pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_rows*pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_cols));
    close(fd);
    dump_table("motion_comp_pc_table", m_pu8tDynamicNRTbl_Motion_COMP_PC, pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_rows, pqconfig.m_u8tDynamicNRTbl_Motion_COMP_PC_cols);
    fd = open("/proc/mi_modules/common/misc_param_table", O_RDONLY);
    error_wrapper(fd);
    MI_U8 tDynamicNRTbl_MISC_Param[pqconfig.m_u8tDynamicNRTbl_MISC_Param_rows*pqconfig.m_u8tDynamicNRTbl_MISC_Param_cols];
    error_wrapper(read(fd, tDynamicNRTbl_MISC_Param, pqconfig.m_u8tDynamicNRTbl_MISC_Param_rows*pqconfig.m_u8tDynamicNRTbl_MISC_Param_cols));
    close(fd);
    dump_table("misc_param_table", tDynamicNRTbl_MISC_Param, pqconfig.m_u8tDynamicNRTbl_MISC_Param_rows, pqconfig.m_u8tDynamicNRTbl_MISC_Param_cols);
    fd = open("/proc/mi_modules/common/misc_luma_table", O_RDONLY);
    error_wrapper(fd);
    MI_U8 tDynamicNRTbl_MISC_Luma[pqconfig.m_u8tDynamicNRTbl_MISC_Luma_rows*pqconfig.m_u8tDynamicNRTbl_MISC_Luma_cols];
    error_wrapper(read(fd, tDynamicNRTbl_MISC_Luma, pqconfig.m_u8tDynamicNRTbl_MISC_Luma_rows*pqconfig.m_u8tDynamicNRTbl_MISC_Luma_cols));
    close(fd);
    dump_table("misc_luma_table", tDynamicNRTbl_MISC_Luma, pqconfig.m_u8tDynamicNRTbl_MISC_Luma_rows, pqconfig.m_u8tDynamicNRTbl_MISC_Luma_cols);
    fd = open("/proc/mi_modules/common/noise_table", O_RDONLY);
    error_wrapper(fd);
    MI_U8 m_pu8tDynamicNRTbl_Noise[pqconfig.m_u8tDynamicNRTbl_Noise_rows*pqconfig.m_u8tDynamicNRTbl_Noise_cols];
    error_wrapper(read(fd, m_pu8tDynamicNRTbl_Noise, pqconfig.m_u8tDynamicNRTbl_Noise_rows*pqconfig.m_u8tDynamicNRTbl_Noise_cols));
    close(fd);
    dump_table("noise_table", m_pu8tDynamicNRTbl_Noise, pqconfig.m_u8tDynamicNRTbl_Noise_rows, pqconfig.m_u8tDynamicNRTbl_Noise_cols);
    fd = open("/proc/mi_modules/common/misc_table", O_RDONLY);
    error_wrapper(fd);
    MI_U8 tDynamicNR_MISC_Noise[pqconfig.m_u8tDynamicNR_MISC_Noise_rows*pqconfig.m_u8tDynamicNR_MISC_Noise_cols];
    error_wrapper(read(fd, tDynamicNR_MISC_Noise, pqconfig.m_u8tDynamicNR_MISC_Noise_rows*pqconfig.m_u8tDynamicNR_MISC_Noise_cols));
    close(fd);
    dump_table("misc_table", tDynamicNR_MISC_Noise, pqconfig.m_u8tDynamicNR_MISC_Noise_rows, pqconfig.m_u8tDynamicNR_MISC_Noise_cols);
}
