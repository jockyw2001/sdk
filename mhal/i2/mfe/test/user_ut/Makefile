
CC := $(CROSS_COMPILE)gcc
EC := echo
RM := rm
CP := cp

KERNEL_DIR := ../../../../linux-3.18

CFLAGS := -Werror -Wall -I../../drv/inc -I../../drv/pub -I$(KERNEL_DIR)/drivers/mstar/include -I$(KERNEL_DIR)/drivers/mstar/camdriver/include -I$(PWD) -lpthread
CFLAGS += -I$(KERNEL_DIR)/drivers/mstar/cam_os_wrapper/pub
CFLAGS += -DCAM_OS_LINUX_USER -ldl

.PHONY: clean

all: mmfem

mmfem: mfem.c mfe_utility.c md5.c $(KERNEL_DIR)/drivers/mstar/cam_os_wrapper/src/cam_os_wrapper.c $(KERNEL_DIR)/drivers/mstar/cam_os_wrapper/src/cam_os_informal_idr.c
	@$(EC) building mmfem ...
	@$(CC) $(CFLAGS) mfe_utility.c  md5.c mfem.c $(KERNEL_DIR)/drivers/mstar/cam_os_wrapper/src/cam_os_wrapper.c $(KERNEL_DIR)/drivers/mstar/cam_os_wrapper/src/cam_os_informal_idr.c -o mmfem

clean:
	@$(RM) -f mmfem
