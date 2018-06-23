include raw-impl-config.mk
DEPS:=simple new_drv
DRV_PUB_INCS:=api/api.h
DRV_INC_PATH:=api
DRV_SRCS:=mjpe/mjpe.c mvd/mvd.c hvd/hvd.c

HAL_PUB_PATH:=mvd vpu hvd
HAL_INC_PATH:=mvd vpu hvd
HAL_SRCS:=mvd/mvd.c vpu/vpu.c hvd/hvd.c
include add-config.mk
