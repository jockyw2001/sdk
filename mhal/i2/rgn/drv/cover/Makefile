#
# Makefile for MStar fb device drivers.

MS_ARCH_NAME=infinity2

# general options
EXTRA_CFLAGS                +=  -Idrivers/mstar/include
#--------gop--------
EXTRA_CFLAGS                +=  -Idrivers/mstar/cover/hal/infinity2/inc
EXTRA_CFLAGS                +=  -Idrivers/mstar/cover/hal/infinity2/src
EXTRA_CFLAGS                +=  -Idrivers/mstar/cover/drv/pub
EXTRA_CFLAGS                +=  -Idrivers/mstar/cover/drv/src
EXTRA_CFLAGS                +=  -Idrivers/mstar/cmdq/drv/pub
EXTRA_CFLAGS                +=  -Idrivers/mstar/cam_os_wrapper/pub
EXTRA_CFLAGS                +=  -Idrivers/mstar/rgn/MI_HAL/pub

EXTRA_CFLAGS      +=  -DCOVEROS_TYPE_LINUX_KERNEL

# files

obj-$(CONFIG_MS_COVER_INFINITY2)   += cover.o

cover-objs := ../hal/infinity2/src/hal_cover.o \
                src/drv_cover.o