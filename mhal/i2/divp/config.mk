include hal-impl-config.mk
DEPS:=cmdq_service common divp

DRV_PUB_INCS:=mhal_divp.h mhal_divp_datatype.h
DRV_INC_PATH:=

DRV_SRCS:=mdrv_divp.c divp_proc.c

HAL_SRCS:=mhal_dip.c
include add-config.mk
