include hal-impl-config.mk
MACROS:=
DEPS:= common venc

DRV_PUB_INCS:= mhal_warp.h
DRV_SRCS:= common/dev_warp.c \
           common/mhal_warp.c

HAL_SRCS:= hal_warp.c \
           hal_ceva.c \
           hal_clk.c
include add-config.mk
