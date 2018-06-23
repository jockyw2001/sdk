






typedef struct mi_vif_ChnAttr_s
{
    MI_BOOL bEnable;
    MI_U32 u32VbFail;
    MI_SYS_BufDataType_e eBufType;
    MI_SYS_PixelFormat_e eFormat;
    MI_VIF_ChnPortAttr_t stChnPortAttr;
    MI_SYS_ChnPort_t stSysPort;
} mi_vif_PortContex_t;

typedef struct mi_vif_ChnAttr_s
{
    mi_vif_PortContex_t stPortCtx[2];
} mi_vif_ChnContex_t;

typedef struct mi_vif_DevContex_s
{
    MI_VIF_DevAttr_t stDevAttr;
    MI_BOOL bEnable;
} mi_vif_DevContex_t;

