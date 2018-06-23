#
# Makefile for MStar JPE (Multi-Format Encoder) device driver.

obj-m       := jpe_burnin_ut.o
jpe_burnin_ut-objs := jpe_ut.o \
                      file_access.o md5.o

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

KERNEL = ../../../../linux-3.18
CC  = $(CROSS_COMPILE)gcc
APP = jpe_burnin_ut
# general options
EXTRA_CFLAGS += -I$(PWD)/../../drv/pub
EXTRA_CFLAGS += -I$(PWD)/../../drv/inc
EXTRA_CFLAGS += -I$(PWD)/$(KERNEL)/drivers/mstar/include
EXTRA_CFLAGS += -I$(PWD)/$(KERNEL)/drivers/mstar/camdriver/include

all:
	make modules -C $(KERNEL)  M=`pwd`

clean:
	make modules clean -C $(KERNEL)  M=`pwd`
	rm -rf $(APP)