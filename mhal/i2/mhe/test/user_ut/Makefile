
CC := $(CROSS_COMPILE)gcc
EC := echo
RM := rm
CP := cp

KERNEL = ../../../../linux-3.18
MHE_DIR = ${KERNEL}/drivers/mstar/camdriver/mhe

CFLAGS := -Werror -Wall -lpthread
CFLAGS += -I$(MHE_DIR)/drv/pub
CFLAGS += -I$(MHE_DIR)/drv/inc
CFLAGS += -I$(KERNEL)/drivers/mstar/include
CFLAGS += -I$(KERNEL)/drivers/mstar/camdriver/include
CFLAGS += -I$(KERNEL)/include/mstar
CFLAGS += -I$(KERNEL)/drivers/mstar/cam_os_wrapper/pub
CFLAGS += -DCAM_OS_LINUX_USER -ldl
#CFLAGS += -DMHE_MSRC_SIMULATION

.PHONY: clean

all: mmhem

mmhem: mhem.c mhe_utility.c mhe_control.c md5.c $(KERNEL)/drivers/mstar/cam_os_wrapper/src/cam_os_wrapper.c
	@$(EC) building mmhem ...
	@$(CC) $(CFLAGS) mhe_utility.c mhe_control.c md5.c mhem.c $(KERNEL)/drivers/mstar/cam_os_wrapper/src/cam_os_wrapper.c $(KERNEL)/drivers/mstar/cam_os_wrapper/src/cam_os_informal_idr.c -o mmhem

clean:
	@$(RM) -f mmhem
