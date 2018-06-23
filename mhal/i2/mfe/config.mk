include hal-impl-config.mk
MACROS:= ENABLE_DUMP_REG \
         SUPPORT_CMDQ_SERVICE #\
         MFE_DISABLE_PMBR #\
         SIMULATE_ON_I3
DEPS:= common venc cmdq_service

DRV_PUB_INCS:= mhal_mfe.h
DRV_SRCS:= common/drv_mfe_ctx.c \
           common/drv_mfe_dev.c \
           common/mhal_mfe.c \
           common/mhal_mfe_dc.c \
           linux/drv_mfe_clk.c \
           linux/drv_mfe_proc.c \
           linux/drv_mfe_export.c

HAL_SRCS:= hal_h264_enc.c \
           hal_mfe_ios.c \
           hal_mfe_ops.c \
           hal_mfe_pmbr.c \
           hal_mfe_rqc.c \
           hal_msb2_rqc.c \
           hal_mfe_global.c \
           hal_mfe_msmath.c \
           hal_msrc_rqc.c
include add-config.mk
