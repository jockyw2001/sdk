include hal-impl-config.mk
MACROS:=
DEPS:= common venc

DRV_PUB_INCS:= mhal_jpe.h
DRV_SRCS:= common/drv_jpe_ctx.c \
           common/drv_jpe_dev.c \
           common/jpegenc_marker.c \
           common/mhal_jpe.c \
           linux/drv_jpe_clk.c \
           linux/drv_jpe_export.c

HAL_SRCS:= hal_jpe.c \
           hal_jpe_ios.c \
           hal_jpe_ops.c
include add-config.mk
