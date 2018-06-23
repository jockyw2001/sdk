#include "mi_syscfg.h"
#include "mi_common_internal.h"
#include "mi_sys_proc_fs_internal.h"
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/kmod.h>

static MI_SYSCFG_Config_t _stConfig;
static MI_SYSCFG_PQConfig_t _stPQConfig;

MI_U8 MI_SYSCFG_GetDLCTableCount(void){
    return _stConfig.m_u8DLCTableCount;
}
EXPORT_SYMBOL(MI_SYSCFG_GetDLCTableCount);

PanelInfo_t* MI_SYSCFG_GetPanelInfo(EN_DISPLAYTIMING_RES_TYPE eTiming, MI_BOOL bHdmiTx)
{
    MI_U8 u8Count = 0;    
    for(u8Count = 0; u8Count < _stConfig.m_PanelConf.nSize; u8Count++)
    {
        if(_stConfig.m_pPanelInfo[u8Count].eTiming == eTiming)
        {
            printk("[%s %d] eTiming = %d, hdmiTx = %d \n", __FUNCTION__, __LINE__, _stConfig.m_pPanelInfo[u8Count].eTiming, _stConfig.m_pPanelInfo[u8Count].bHdmiTx == true);
            return &(_stConfig.m_pPanelInfo[u8Count]);
        }
    }
    
    printk("[%s %d] eTiming = %d, hdmiTx = %d Not Fund!!!\n", __FUNCTION__, __LINE__, eTiming, bHdmiTx);
    return NULL;
}
EXPORT_SYMBOL(MI_SYSCFG_GetPanelInfo);


const int _pqBaseSize = offsetof(MI_SYSCFG_PQConfig_t, m_pu8tDynamicNRTbl_Noise);

static MI_S32 MI_SYSCFG_ConfigGet(char *buf, MI_U64 off, MI_S32 v){
    memcpy(buf, ((void*)&_stConfig)+off, v);
    return v;
}

static MI_S32 MI_SYSCFG_ConfigSet(const char *buf, MI_U64 off, MI_S32 v){
    memcpy(((void*)&_stConfig)+off, buf, v);
    return v;
}

static MI_S32 MI_SYSCFG_PQConfigGet(char *buf, MI_U64 off, MI_S32 v){
    if(v < _pqBaseSize){
        return -EINVAL;
    }
    memcpy(buf, &_stPQConfig, _pqBaseSize);
    return v;
}

static MI_S32 MI_SYSCFG_PQConfigSet(const char *buf, MI_U64 off, MI_S32 v){
    if(v < _pqBaseSize){
        return -EINVAL;
    }
    memcpy(&_stPQConfig, buf, _pqBaseSize);
    return v;
}

static MI_S32 MI_SYSCFG_PQNoiseTableGet(char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_Noise_rows*_stPQConfig.m_u8tDynamicNRTbl_Noise_cols)
        return -EINVAL;
    memcpy(buf, _stPQConfig.m_pu8tDynamicNRTbl_Noise, v);
    return v;
}

static MI_S32 MI_SYSCFG_PQNoiseTableSet(const char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_Noise_rows*_stPQConfig.m_u8tDynamicNRTbl_Noise_cols)
        return -EINVAL;
    kfree(_stPQConfig.m_pu8tDynamicNRTbl_Noise);
    _stPQConfig.m_pu8tDynamicNRTbl_Noise = kmemdup(buf, v, GFP_KERNEL);
    return v;
}

static MI_S32 MI_SYSCFG_PQMotionTableGet(char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_Motion_rows*_stPQConfig.m_u8tDynamicNRTbl_Motion_cols)
        return -EINVAL;
    memcpy(buf, _stPQConfig.m_pu8tDynamicNRTbl_Motion, v);
    return v;
}

static MI_S32 MI_SYSCFG_PQMotionTableSet(const char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_Motion_rows*_stPQConfig.m_u8tDynamicNRTbl_Motion_cols)
        return -EINVAL;
    kfree(_stPQConfig.m_pu8tDynamicNRTbl_Motion);
    _stPQConfig.m_pu8tDynamicNRTbl_Motion = kmemdup(buf, v, GFP_KERNEL);
    return v;
}

static MI_S32 MI_SYSCFG_PQMotionHdmiDtvTableGet(char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows*_stPQConfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols)
        return -EINVAL;
    memcpy(buf, _stPQConfig.m_pu8tDynamicNRTbl_Motion_HDMI_DTV, v);
    return v;
}

static MI_S32 MI_SYSCFG_PQMotionHdmiDtvTableSet(const char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows*_stPQConfig.m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols)
        return -EINVAL;
    kfree(_stPQConfig.m_pu8tDynamicNRTbl_Motion_HDMI_DTV);
    _stPQConfig.m_pu8tDynamicNRTbl_Motion_HDMI_DTV = kmemdup(buf, v, GFP_KERNEL);
    return v;
}

static MI_S32 MI_SYSCFG_PQMotionCompPcTableGet(char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_Motion_COMP_PC_rows*_stPQConfig.m_u8tDynamicNRTbl_Motion_COMP_PC_cols)
        return -EINVAL;
    memcpy(buf, _stPQConfig.m_pu8tDynamicNRTbl_Motion_COMP_PC, v);
    return v;
}

static MI_S32 MI_SYSCFG_PQMotionCompPcTableSet(const char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_Motion_COMP_PC_rows*_stPQConfig.m_u8tDynamicNRTbl_Motion_COMP_PC_cols)
        return -EINVAL;
    kfree(_stPQConfig.m_pu8tDynamicNRTbl_Motion_COMP_PC);
    _stPQConfig.m_pu8tDynamicNRTbl_Motion_COMP_PC = kmemdup(buf, v, GFP_KERNEL);
    return v;
}

static MI_S32 MI_SYSCFG_PQMiscTableGet(char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNR_MISC_Noise_rows*_stPQConfig.m_u8tDynamicNR_MISC_Noise_cols)
        return -EINVAL;
    memcpy(buf, _stPQConfig.tDynamicNR_MISC_Noise, v);
    return v;
}

static MI_S32 MI_SYSCFG_PQMiscTableSet(const char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNR_MISC_Noise_rows*_stPQConfig.m_u8tDynamicNR_MISC_Noise_cols)
        return -EINVAL;
    kfree(_stPQConfig.tDynamicNR_MISC_Noise);
    _stPQConfig.tDynamicNR_MISC_Noise = kmemdup(buf, v, GFP_KERNEL);
    return v;
}

static MI_S32 MI_SYSCFG_PQMiscLumaTableGet(char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_MISC_Luma_rows*_stPQConfig.m_u8tDynamicNRTbl_MISC_Luma_cols)
        return -EINVAL;
    memcpy(buf, _stPQConfig.tDynamicNRTbl_MISC_Luma, v);
    return v;
}

static MI_S32 MI_SYSCFG_PQMiscLumaTableSet(const char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_MISC_Luma_rows*_stPQConfig.m_u8tDynamicNRTbl_MISC_Luma_cols)
        return -EINVAL;
    kfree(_stPQConfig.tDynamicNRTbl_MISC_Luma);
    _stPQConfig.tDynamicNRTbl_MISC_Luma = kmemdup(buf, v, GFP_KERNEL);
    return v;
}

static MI_S32 MI_SYSCFG_PQMiscParamTableGet(char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_MISC_Param_rows*_stPQConfig.m_u8tDynamicNRTbl_MISC_Param_cols)
        return -EINVAL;
    memcpy(buf, _stPQConfig.tDynamicNRTbl_MISC_Param, v);
    return v;
}

static MI_S32 MI_SYSCFG_PQMiscParamTableSet(const char *buf, MI_U64 off, MI_S32 v){
    if(v != _stPQConfig.m_u8tDynamicNRTbl_MISC_Param_rows*_stPQConfig.m_u8tDynamicNRTbl_MISC_Param_cols)
        return -EINVAL;
    kfree(_stPQConfig.tDynamicNRTbl_MISC_Param);
    _stPQConfig.tDynamicNRTbl_MISC_Param = kmemdup(buf, v, GFP_KERNEL);
    return v;
}

void MI_SYSCFG_SetupIniLoader(void){
    char *argv[] = {"load_config", NULL};
    char *envp[] = {NULL};
    
    MI_COMMON_AddDebugRawFile("config_info", /*(sizeof(MI_SYSCFG_Config_t), */MI_SYSCFG_ConfigSet, MI_SYSCFG_ConfigGet);
    MI_COMMON_AddDebugRawFile("pq_info", /*_pqBaseSize, */MI_SYSCFG_PQConfigSet, MI_SYSCFG_PQConfigGet);
    MI_COMMON_AddDebugRawFile("noise_table", /*4096, */MI_SYSCFG_PQNoiseTableSet, MI_SYSCFG_PQNoiseTableGet);
    MI_COMMON_AddDebugRawFile("motion_table", /*4096, */MI_SYSCFG_PQMotionTableSet, MI_SYSCFG_PQMotionTableGet);
    MI_COMMON_AddDebugRawFile("motion_hdmi_dtv_table", /*4096,*/ MI_SYSCFG_PQMotionHdmiDtvTableSet, MI_SYSCFG_PQMotionHdmiDtvTableGet);
    MI_COMMON_AddDebugRawFile("motion_comp_pc_table", /*4096,*/ MI_SYSCFG_PQMotionCompPcTableSet, MI_SYSCFG_PQMotionCompPcTableGet);
    MI_COMMON_AddDebugRawFile("misc_table", /*4096, */MI_SYSCFG_PQMiscTableSet, MI_SYSCFG_PQMiscTableGet);
    MI_COMMON_AddDebugRawFile("misc_luma_table", /*4096, */MI_SYSCFG_PQMiscLumaTableSet, MI_SYSCFG_PQMiscLumaTableGet);
    MI_COMMON_AddDebugRawFile("misc_param_table", /*4096, */MI_SYSCFG_PQMiscParamTableSet, MI_SYSCFG_PQMiscParamTableGet);

    call_usermodehelper("/config/config_tool", argv, envp, UMH_WAIT_PROC);
}
