BASEDIR?=$(CURDIR)/..

INCS =  mstar2/hal/$(CONFIG_MSTAR_CHIP_NAME)/cpu/include/mstar \
        $(PROJ_ROOT)/../sdk/mhal/include/utopia \
        $(PROJ_ROOT)/../sdk/interface/include/common \
        $(PROJ_ROOT)/../sdk/interface/include/sys \
        $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/uapi

SRCS =  iniparser.o  mdrv_graphic_adapter.o mdrv_graphic_fbdev.o

include $(BASEDIR)/kbuild.mk
