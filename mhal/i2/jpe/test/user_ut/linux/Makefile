
CC := $(CROSS_COMPILE)gcc
EC := echo
RM := rm
CP := cp

KERNEL_DIR := ../../../../../../kernel/linux-3.18

CFLAGS := -Werror -Wall -I../../../drv/inc -I../../../drv/pub -I../common -I$(KERNEL_DIR)/drivers/mstar/include -I$(PWD) -lpthread
CFLAGS += -DCAM_OS_LINUX_USER -ldl

.PHONY: clean

all: jpe_ut

jpe_ut: jpe_ut.c ../common/jpegenc_marker.c $(KERNEL_DIR)/drivers/mstar/cam_os_wrapper/src/cam_os_wrapper.c $(KERNEL_DIR)/drivers/mstar/cam_os_wrapper/src/cam_os_informal_idr.c
	@$(EC) building jpe_ut ...
	@$(CC) $(CFLAGS) jpe_ut.c ../common/jpegenc_marker.c $(KERNEL_DIR)/drivers/mstar/cam_os_wrapper/src/cam_os_wrapper.c $(KERNEL_DIR)/drivers/mstar/cam_os_wrapper/src/cam_os_informal_idr.c -o jpe_ut

clean:
	@$(RM) -f jpe_ut
