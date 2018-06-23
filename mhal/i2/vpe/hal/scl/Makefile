MS_SCL_ARCH_NAME=infinity2
MS_SCL_OS_NAME=linux
MS_SCL_SRC_PATH=hal/$(MS_SCL_ARCH_NAME)/scl/src

EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/include
EXTRA_CFLAGS      +=  -Idrivers/mstar/include
#EXTRA_CFLAGS      +=  -Idrivers/mstar/include/infinity3
#
# Makefile for MStar SCL drivers.
#


# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/drv/MI_HAL/pub
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/drv/MI_HAL/inc
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/drv/isp/pub
EXTRA_CFLAGS      +=  -Idrivers/mstar/driver/drv/mload/drv/pub
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/drv/scl/pub
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/drv/scl/pub/$(MS_SCL_OS_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/drv/scl/inc
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/drv/scl/inc/pq
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/drv/scl/inc/$(MS_SCL_OS_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/hal/$(MS_SCL_ARCH_NAME)/scl/inc
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/hal/$(MS_SCL_ARCH_NAME)/scl/inc/drv
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/hal/$(MS_SCL_ARCH_NAME)/scl/inc/pq
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/hal/$(MS_SCL_ARCH_NAME)/scl/inc/drv/$(MS_SCL_OS_NAME)
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/hal/$(MS_SCL_ARCH_NAME)/scl/inc/hal
EXTRA_CFLAGS      +=  -Idrivers/mstar/camdriver/vpe/hal/$(MS_SCL_ARCH_NAME)/scl/inc/hal/$(MS_SCL_OS_NAME)

EXTRA_CFLAGS += -Werror

# specific options
#

EXTRA_CFLAGS      +=  -DSCLOS_TYPE_LINUX_KERNEL
ifdef CONFIG_MS_SCL_INFINITY2
#EXTRA_CFLAGS      +=  -DSCLOS_TYPE_LINUX_KERNEL_I2 -DSCLOS_TYPE_LINUX_TEST -DSCLOS_TYPE_LINUX_TESTCMDQ -DSCLOS_TYPE_LINUX_PLATFORMDEVICE
EXTRA_CFLAGS      +=  -DSCLOS_TYPE_LINUX_KERNEL_I2 -DSCLOS_TYPE_LINUX_PLATFORMDEVICE
endif
ifdef CONFIG_MS_MLOAD
#EXTRA_CFLAGS      +=  -DSCLOS_TYPE_MLOAD
endif
ifdef CONFIG_MS_SCL_USBCAM
EXTRA_CFLAGS      +=  -DUSE_USBCAM
endif
ccflags-$(CONFIG_ARCH_CEDRIC)		+= -DC3_SIM


obj-$(CONFIG_MS_SCL_INFINITY2)   += mdrv_scl.o
obj-$(CONFIG_MS_SCL_INFINITY3)   += mdrv_scl.o

mdrv_scl-y :=     src/drv/drv_scl_dma_io_wrapper.o \
                  src/drv/drv_scl_hvsp_io_wrapper.o \
                  src/drv/drv_scl_ctx_m.o \
                  src/hal/hal_scl_irq.o \
                  src/hal/hal_utility.o\
                  src/hal/hal_scl_hvsp.o \
                  src/hal/hal_scl_dma.o \
                  src/hal/hal_scl_vip.o \
                  src/pq/hal_scl_pq_qualitymap_main.o \
                  src/drv/drv_scl_hvsp.o \
                  src/drv/drv_scl_dma.o \
                  src/drv/drv_scl_vip.o \
                  src/drv/drv_scl_hvsp_m.o \
                  src/drv/drv_scl_dma_m.o \
                  src/drv/drv_scl_vip_m.o \
                  src/drv/drv_scl_m2m_io_wrapper.o \
                  src/drv/drv_scl_vip_io_wrapper.o \
                  src/drv/$(MS_SCL_OS_NAME)/drv_scl_irq.o \
                  ../../../drv/scl/src/pq/drv_scl_pq.o \
                  ../../../drv/scl/src/$(MS_SCL_OS_NAME)/drv_scl_os.o \
                  ../../../drv/MI_HAL/src/mhal_vpe.o \
                  ../../../drv/scl/src/$(MS_SCL_OS_NAME)/drv_scl_ioctl_parse.o \
                  ../../../drv/scl/src/ut/scl_ut.o \
                  ../../../drv/scl/src/$(MS_SCL_OS_NAME)/drv_scl_module.o
