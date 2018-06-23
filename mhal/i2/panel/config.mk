include raw-impl-config.mk
DEPS:=panel
MACROS:=PNL_OS_TYPE_LINUX_KERNEL

DRV_PUB_INCS:=mihal/pub/mhal_pnl.h  mihal/pub/mhal_pnl_datatype.h
DRV_INC_PATH:=pnl/pub
DRV_INC_PATH+=../hal/pnl/inc/drv
DRV_INC_PATH+=mihal
DRV_INC_PATH+=mihal/inc
DRV_INC_PATH+=mihal/pub
DRV_INC_PATH+=../../../include/common

DRV_SRCS:=../hal/pnl/src/drv/drv_pnl.c
DRV_SRCS+=../hal/pnl/src/drv/drv_pnl_io_wrapper.c
DRV_SRCS+=mihal/src/mhal_pnl.c

HAL_PUB_PATH:=pnl/inc/hal
HAL_INC_PATH:=pnl/inc/hal
HAL_INC_PATH+=pnl/inc/drv

HAL_SRCS:=pnl/src/hal/hal_pnl.c
HAL_SRCS+=pnl/src/hal/hal_pnl_lpll_tbl.c

include add-config.mk
