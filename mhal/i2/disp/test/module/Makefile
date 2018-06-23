#
# Makefile for kernel test
#
PWD         ?= $(shell pwd)
DIR_PATH    := $(PWD)
KVERSION    := $(shell uname -r)
KERNEL_DIR   = $(PWD)/../../../../../../project/kbuild/3.18.30/

BUILD_MHAL_DISP ?=n
BUILD_MHAL_HDMITX  ?=y

MODULE_NAME  = mhal_disp

INC_PATH := -I$(DIR_PATH)/
INC_PATH += -I$(DIR_PATH)/inc
INC_PATH += -I$(DIR_PATH)/../../drv/mihal/inc
INC_PATH += -I$(DIR_PATH)/../../drv/mgwin/pub
INC_PATH += -I$(DIR_PATH)/../../drv/mihal/pub
INC_PATH += -I$(DIR_PATH)/../../hal/mgwin/inc/drv
INC_PATH += -I$(DIR_PATH)/../../drv/mgwin/pub
INC_PATH += -I$(DIR_PATH)/../../hal/mgwin/inc/hal
INC_PATH += -I$(DIR_PATH)/../../hal/mgwin/inc/drv
INC_PATH += -I$(DIR_PATH)/hdmitx

obj-m       := $(MODULE_NAME).o
$(MODULE_NAME)-objs := SimMiModule.o
$(MODULE_NAME)-objs += SimMiModuleDisp.o

ifeq ($(BUILD_MHAL_DISP),y)
	$(MODULE_NAME)-objs += ../../drv/mihal/src/mhal_disp.o
	$(MODULE_NAME)-objs += ../../drv/mihal/src/mhal_pql.o
	$(MODULE_NAME)-objs += ../../drv/mihal/src/mhal_disp_pnl_tbl.o
	$(MODULE_NAME)-objs += ../../hal/mgwin/src/hal/hal_scl_mgwin.o
	$(MODULE_NAME)-objs += ../../hal/mgwin/src/drv/drv_scl_mgwin.o
	$(MODULE_NAME)-objs += ../../hal/mgwin/src/drv/drv_scl_mgwin_io_wrapper.o
else
	EXTRA_CFLAGS += -DMI_ENABLE
endif

ifeq ($(BUILD_MHAL_HDMITX),y)
	$(MODULE_NAME)-objs += ./hdmitx/mhal_hdmitx.o
	EXTRA_CFLAGS += -DMHAL_HDMITX_EN
endif

EXTRA_CFLAGS += -DMSOS_TYPE_LINUX_KERNEL -DSCLOS_TYPE_LINUX_KERNEL -DSCLOS_TYPE_LINUX_TEST -DCHIP_I2
EXTRA_CFLAGS += $(INC_PATH)

all:
	@echo $(INC_PATH)
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
