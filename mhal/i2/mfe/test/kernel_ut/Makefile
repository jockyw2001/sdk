#
# Makefile for MStar MFE (Multi-Format Encoder) kernel mode unittest.

obj-m       := mfe_kernel_ut.o
mfe_kernel_ut-objs :=   mfe_ut.o file_access.o

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

KERNEL = ../../../../../kernel/linux-3.18
CC  = $(CROSS_COMPILE)gcc
APP = mfe_kernel_ut
# general options
EXTRA_CFLAGS += -I$(PWD)/../../../mfe/drv/pub
EXTRA_CFLAGS += -I$(PWD)/../../../mhe/drv/pub
EXTRA_CFLAGS += -I$(PWD)/../../drv/inc
EXTRA_CFLAGS += -I$(PWD)/$(KERNEL)/drivers/mstar/include
EXTRA_CFLAGS += -I$(PWD)/$(KERNEL)/drivers/mstar/camdriver/include
EXTRA_CFLAGS += -DSUPPORT_CMDQ_SERVICE

all:
	make modules -C $(KERNEL)  M=`pwd`

clean:
	make modules clean -C $(KERNEL)  M=`pwd`
	rm -rf $(APP)
