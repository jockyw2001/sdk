#
# Makefile for kernel test
#
PWD         ?= $(shell pwd)
DIR_PATH    := $(PWD)
KVERSION    := $(shell uname -r)
KERNEL_DIR   = $(PWD)/../../../../../../project/kbuild/3.18.30/

MODULE_NAME  = mhal_pnl

INC_PATH := -I$(DIR_PATH)/
INC_PATH += -I$(DIR_PATH)/inc
INC_PATH += -I$(DIR_PATH)/../../drv/mihal/inc
INC_PATH += -I$(DIR_PATH)/../../drv/mgwin/pub
INC_PATH += -I$(DIR_PATH)/../../drv/mihal/pub
INC_PATH += -I$(DIR_PATH)/../../hal/pnl/inc/drv
INC_PATH += -I$(DIR_PATH)/../../drv/pnl/pub
INC_PATH += -I$(DIR_PATH)/../../hal/pnl/inc/hal
INC_PATH += -I$(DIR_PATH)/../../hal/pnl/inc/drv


obj-m       := $(MODULE_NAME).o
$(MODULE_NAME)-objs := SimMiModule.o
$(MODULE_NAME)-objs += SimMiModulePnl.o
$(MODULE_NAME)-objs += SimMiModulePnlTbl.o


EXTRA_CFLAGS += -DCHIP_I2 -DPNL_OS_TYPE_LINUX
EXTRA_CFLAGS += $(INC_PATH)

all:
	@echo $(INC_PATH)
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
