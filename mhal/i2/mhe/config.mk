include hal-impl-config.mk
MACROS:= ENABLE_DUMP_REG \
         SUPPORT_CMDQ_SERVICE

DEPS:= common venc cmdq_service

DRV_PUB_INCS:= mhal_mhe.h
DRV_SRCS:= common/drv_mhe_ctx.c \
           common/drv_mhe_dev.c \
           common/mhal_mhe.c \
           linux/drv_mhe_rqc.c \
           linux/drv_mhe_clk.c \
           linux/drv_mhe_proc.c \
           linux/drv_mhe_export.c

HAL_SRCS:= hal_h265_enc.c \
           hal_mhe_ios.c \
           hal_mhe_ops.c \
           hal_mhe_pmbr.c \
           hal_mhe_rqc.c \
           hal_mhe_global.c \
           hal_mhe_msmath.c \
           hal_msrc_rqc.c

include add-config.mk