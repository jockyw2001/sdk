#
# Makefile for MStar MHE (Multi-Format Encoder) device driver.

obj-m       := mhe_burnin_ut.o
mhe_burnin_ut-objs :=  mhe_ut.o file_access.o md5.o

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

KERNEL = ../../../../../kernel/linux-3.18/
MHE_DIR = ${KERNEL}/drivers/mstar/camdriver/mhe

#CC  = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
APP = mhe_burnin_ut
# general options
EXTRA_CFLAGS += -I$(PWD)/../../drv/pub
EXTRA_CFLAGS += -I$(PWD)/../../drv/inc
EXTRA_CFLAGS += -I$(PWD)/$(KERNEL)/drivers/mstar/include
EXTRA_CFLAGS += -I$(PWD)/$(KERNEL)/drivers/mstar/camdriver/include
EXTRA_CFLAGS += -DSUPPORT_CMDQ_SERVICE

all:
	make modules -C $(KERNEL)  M=`pwd`
	arm-linux-gnueabihf-strip --strip-debug *.ko

clean:
	make modules clean -C $(KERNEL)  M=`pwd`
	rm -rf $(APP)
