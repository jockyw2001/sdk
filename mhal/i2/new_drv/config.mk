include hal-impl-config.mk
MACROS:=A B C
DEPS:=simple raw_drv

DRV_PUB_INCS:=new_pub.h
DRV_SRCS:=src1.c src2.c

HAL_SRCS:=src1.c src2.c
include add-config.mk
