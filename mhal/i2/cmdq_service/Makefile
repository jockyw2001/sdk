#
# Makefile for MStar dip device drivers.

# general options
EXTRA_CFLAGS                +=  -Idrivers/mstar/include
EXTRA_CFLAGS                +=  -Idrivers/mstar/include/infinity2

#----------------------- msos -----------------------
#EXTRA_CFLAGS                +=  -Idrivers/mstar/dip/drv/inc/msos

#----------------------- drv -----------------------
EXTRA_CFLAGS                +=  -Idrivers/mstar/camdriver/cmdq
EXTRA_CFLAGS                +=  -Idrivers/mstar/camdriver/cmdq/drv/inc
EXTRA_CFLAGS                +=  -Idrivers/mstar/camdriver/cmdq/drv/pub
EXTRA_CFLAGS                +=  -Idrivers/mstar/camdriver/cmdq/drv/inc/linux
#----------------------- hal -----------------------
EXTRA_CFLAGS                +=  -Idrivers/mstar/camdriver/cmdq/hal/inc
EXTRA_CFLAGS                +=  -Idrivers/mstar/camdriver/cmdq/hal/pub
EXTRA_CFLAGS                +=  -Idrivers/mstar/camdriver/include

# specific options
EXTRA_CFLAGS                +=

ccflags-y		+= -DHAL_I2_SIMULATE

#--------------------- sources ---------------------
obj-$(CONFIG_MS_CMDQ)   += mstar_cmdq.o
mstar_cmdq-y := drv/src/drvcmdq_io.o  \
                drv/src/linux/drv_cmdq_os.o \
                drv/src/linux/drv_cmdq_irq.o \
                drv/src/linux/cmdq_proc.o \
                drv/src/drv_cmdq.o    \
                hal/src/hal_cmdq.o \
                drv/src/mhal_cmdq.o
