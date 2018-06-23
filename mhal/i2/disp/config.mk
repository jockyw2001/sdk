include raw-impl-config.mk
DEPS:=disp
MACROS:=SCLOS_TYPE_LINUX_KERNEL

DRV_PUB_INCS:=mihal/pub/mhal_disp.h  mihal/pub/mhal_disp_datatype.h
DRV_INC_PATH:=mgwin/pub
DRV_INC_PATH+=../hal/mgwin/inc/drv
DRV_INC_PATH+=mihal
DRV_INC_PATH+=mihal/inc
DRV_INC_PATH+=mihal/pub

DRV_SRCS:=../hal/mgwin/src/drv/drv_scl_mgwin.c
DRV_SRCS+=../hal/mgwin/src/drv/drv_scl_mgwin_io_wrapper.c
DRV_SRCS+=mihal/src/mhal_disp.c
DRV_SRCS+=mihal/src/mhal_disp_pnl_tbl.c
DRV_SRCS+=mihal/src/mhal_pql.c

HAL_PUB_PATH:=mgwin/inc/hal
HAL_INC_PATH:=mgwin/inc/hal
HAL_INC_PATH+=mgwin/inc/drv

HAL_SRCS:=mgwin/src/hal/hal_scl_mgwin.c

include add-config.mk
