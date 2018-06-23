include raw-impl-config.mk
DEPS:=common cmdq_service
MACROS:=GOPOS_TYPE_LINUX_KERNEL COVEROS_TYPE_LINUX_KERNEL

DRV_PUB_INCS:=MI_HAL/pub/mhal_rgn.h MI_HAL/pub/mhal_rgn_datatype.h
DRV_INC_PATH:=gop/pub/ cover/pub/ MI_HAL/pub

DRV_SRCS:=gop/src/drv_gop.c \
cover/src/drv_cover.c \
MI_HAL/src/mhal_rgn.c \

HAL_PUB_PATH:=gop/inc/ cover/inc/
HAL_INC_PATH:=gop/inc/ cover/inc/
HAL_SRCS:=gop/src/hal_gop.c \
cover/src/hal_cover.c
include add-config.mk
