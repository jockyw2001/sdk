#CHIP = k6
CHIP = I2

ifeq ($(CHIP),k6)
CROSS_COMPILE=arm-none-linux-gnueabi-
CONFIG_I2_FPGA := n
else ifeq ($(CHIP),I2)
CROSS_COMPILE=arm-linux-gnueabihf-
CONFIG_I2_FPGA := n
endif

CHIP_NAME = $(shell echo $(CHIP) | tr a-z A-Z)
KDIR := ../../../../../../project/kbuild/3.18.30/


CC := $(CROSS_COMPILE)gcc
EC := echo
RM := rm
CP := cp
ROOT_DIR   = $(PWD)/../..
VDEC_V3_DIR = $(ROOT_DIR)/
EXTNI_INC  = $(ROOT_DIR)/../utpa/mxlib/include


LIBS := -ldl -lpthread

CFLAGS := -I$(ROOT_DIR)/test/mdrv/  \
		   -I$(VDEC_V3_DIR)/drv/api/pub  \
	       -I$(KDIR)/drivers/mstar/include/ \
		   -I$(EXTNI_INC) \
           -D_USE_MDRV_VDEC_ \
           -D'CHIP_$(CHIP_NAME)' \
          -DUFO_PUBLIC_HEADER_700 \

#	       -I$(ROOT_DIR)/extrn/msos/hal/k6/msos/linux/ \
#           -I$(UTOPIA_DIR)/mxlib/include/  \
#           -I$(UTOPIA_DIR)/mxlib/hal/k6/  \
#           -I$(ROOT)/modules/msos/hal/k6/msos/linux \

#ifeq ($(CONFIG_I2_FPGA),y)
#CFLAGS += -DCHIP_I2_FPGA -DCAM_OS_LINUX_USER
#endif

ifeq ($(CHIP),I2)
CFLAGS += -DCAM_OS_LINUX_USER #-DCHIP_I2_FPGA
endif


SRCS = vdec_main.c vdec_test.c

SRCS += libmd5/libmd5.c

ifeq ($(CHIP),I2)  #($(CONFIG_I2_FPGA),y)
SRCS += cam_os_wrapper_s.c
endif

all: vdec_test

vdec_test: $(SRCS)
	@$(EC) building vdec_test ...
	$(CC) $(CFLAGS) ${LIBS} $(SRCS) -o vdec_test

#ifeq ($(CHIP),I2)  #($(CONFIG_I2_FPGA),y)
#vdec_test: vdec_test.c cam_os_wrapper_s.c
#	@$(EC) building vdec_test ...
#	$(CC) $(CFLAGS) ${LIBS} vdec_main.c vdec_test.c cam_os_wrapper_s.c md5.c  -o vdec_test
#else
#vdec_test: vdec_test.c
#	@$(EC) building vdec_test ...
#	$(CC) $(CFLAGS) vdec_main.c vdec_test.c md5.c -o vdec_test  -lpthread
#endif

clean:
	@$(RM) -f vdec_test *.o

.PHONY: clean
