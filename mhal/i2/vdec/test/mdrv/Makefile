#CHIP = k6
CHIP = I2

CONFIG_VDEC_MJPEG = n

ifeq ($(CHIP),k6)
CONFIG_I2_FPGA := n
else ifeq ($(CHIP),I2)
CONFIG_I2_FPGA := y
endif


# **********************************************
# PATH
# **********************************************

ROOT_DIR = $(PWD)/../..
ROOT_VDEC3  = $(ROOT_DIR)/
#EXTNI_INC      = $(ROOT_DIR)/include
OBJ_PATH    = obj
UTOPIA_DIR  = $(ROOT_DIR)/../utpa

CFG_DEFS    = -DMSOS_TYPE_LINUX_KERNEL
#CFG_DEFS   += -DUFO_PUBLIC_HEADER_700
CFG_DEFS   += -D_USE_MDRV_VDEC_ -D_USE_MAPI_IF -D_REWRITE_UTOPIA_FOP

# **********************************************
# define option from Utopia build/Makefile
# **********************************************
CHIP_NAME = $(shell echo $(CHIP) | tr a-z A-Z)
MCU_TYPE = "arm_ca7"

#ifeq ($(CHIP),k6)
#  MCU_TYPE = "arm_ca7"
#  CPU_NAME = armca7
#else ifeq ($(CHIP),I2)
#  MCU_TYPE = "arm_ca9"
#  CPU_NAME = armca9
#endif

########CFG_DEFS += -DCHIP_K6
CFG_DEFS += -D'CHIP_$(CHIP_NAME)' -D'MS_C_STDLIB'
CFG_DEFS += -D'MCU_ARM_CA7'
CFG_DEFS += -D'DONT_USE_CMA'
CFG_DEFS += -D'SUPPORT_EVD=1'
CFG_DEFS += -D'SUPPORT_NEW_MEM_LAYOUT'

# **********************************************
# define option for VDEC3
# **********************************************
CFG_DEFS += -DVDEC3
CFG_DEFS += -DVDEC3_FB
CFG_DEFS += -DSUPPORT_G2VP9=0 -DSUPPORT_MSVP9=0

# **********************************************
# Option to disable some functionalities
# **********************************************
CFG_DEFS  += -D_ENABLE_VDEC_MVD
CFG_DEFS  += -D_DISABLE_MDRV_SYS
CFG_DEFS  += -D_DISABLE_VDEC_MBX
CFG_DEFS  += -D_DISABLE_GETLIBVER

#disable the following options
#CFG_DEFS  += -D_ENABLE_VDEC_SECUMODE
#CFG_DEFS  += -D_ENABLE_VDEC_MJPEG

ifeq ($(CONFIG_I2_FPGA),y)
CFG_DEFS  += -DCHIP_I2_FPGA
endif

# **********************************************
#
# **********************************************
SRCDIR   = \
            $(ROOT_VDEC3)/drv/api/vdec_v3 \
            $(ROOT_VDEC3)/drv/hvd_v3 \
            $(ROOT_VDEC3)/drv/mvd_v3 \
            $(ROOT_VDEC3)/hal/hvd_v3 \
            $(ROOT_VDEC3)/hal/mvd_v3 \
            $(ROOT_VDEC3)/hal/vpu_v3


#SRC_C     = mdrv_vdec.c
#SRC_C    += $(foreach dir, $(SRCDIR),   $(wildcard $(dir)/*.c))
####OBJ_C    = $(notdir $(patsubst %.c, %.o,   $(SRC_C)))
#OBJ_C    = $(notdir $(patsubst %.c, %.o, $(SRC_C)))
#OBJ_C = $(patsubst %.c,${OBJ_PATH}/%.o,$(notdir ${SRC_C}))
#OBJ_C += mdrv_vdec.o

OBJ_C     = mdrv_vdec.o \
             vdec_api.o \


#             ../drv/api/vdec_v3/apiVDEC_EX.o \
             ../drv/hvd_v3/drvHVD_EX.o \
             ../drv/hvd_v3/osalHVD_EX.o \
             ../drv/hvd_v3/HVD_EX_Common.o \
             ../hal/hvd_v3/halHVD_EX.o \
             ../hal/vpu_v3/halVPU_EX.o \
             ../hal/vpu_v3/ms_decompress.o \
             ../drv/mvd_v3/drvMVD_EX.o \
             ../drv/mvd_v3/osalMVD_EX.o \
             ../drv/mvd_v3/mvd_cc_EX.o \
             ../drv/mvd_v3/MVD_EX_Common.o \
             ../hal/mvd_v3/halMVD_EX.o \

#             ../extrn/MsOs.o \
             ../extrn/irq/hal/k6/irq/halIRQ.o \
             ../extrn/msos/drv/mmio/drvMMIO.o \
             ../extrn/msos/hal/k6/mmio/halMMIO.o \
             ../extrn/miu/hal/k6/miu/halMIU.o \


#             ../extrn/msos/msos/$(MCU_TYPE)/asmCPU.o \
#             ../extrn/msos/msos/linux_kernel_V2/MsOS_linux_kernel.o \
#             ../extrn/msos/msos/linux_kernel_V2/mem_priv.o \
#             ../extrn/msos/msos/linux_kernel_V2/drvMPool.o \
#             ../extrn/msos/hal/k6/msos/linux/halMPool.o \

# **********************************************
# disable MJPEG
# **********************************************
ifeq ($(CONFIG_VDEC_MJPEG),y)
SRCDIR   += $(ROOT_VDEC3)/drv/mjpeg_v3
OBJ_C     += ../drv/api/vdec_v3/apiMJPEG_EX.o \
			   ../drv/mjpeg_v3/drvMJPEG_EX.o \
			   ../drv/mjpeg_v3/VirtualhalMJPEG_EX.o \
               ../drv/api/extrn/apiJPEG_priv.o \


CFG_DEFS  +=-D_ENABLE_VDEC_MJPEG
endif

# **********************************************
# .h location
# **********************************************
#API_INCDIR  = $(foreach dir,$(DRV_DIR),$(DDIDRV)/$(dir))
#API_CC_INCS = $(foreach dir,$(API_INCDIR),-I$(dir) -I$(EXTNI_INC))
#PUB_INCS = $(foreach dir,$(SRCDIR),-I$(dir) -I$(EXTNI_INC))
#API_INCS = -I$(UTOPIA_DIR)/projects/build
##################### above: no need


SRC_INCS = $(foreach dir,$(SRCDIR),-I$(dir))

#API_INCS = $(SRC_INCS)
API_INCS =
#API_INCS += -I$(EXTNI_INC) \
			-I$(EXTNI_INC)/hal \
			-I$(EXTNI_INC)/tegra \
            -I$(ROOT_VDEC3)/drv/api/pub \
            -I$(ROOT_VDEC3)/drv/api/inc \
            -I$(ROOT_VDEC3)/hal/inc \
            -I$(ROOT_DIR)/extrn/irq/hal/k6/irq \
            -I$(ROOT_DIR)/extrn/msos/hal/k6/mmio \
            -I$(ROOT_DIR)/extrn/msos/hal/k6/msos/linux \
            -I$(ROOT_DIR)/extrn/msos/utopia_core \
            -I$(ROOT_DIR)/extrn/miu/hal/k6/miu \


API_INCS +=  -I$(ROOT_VDEC3)/drv/api/pub  \
			  -I$(UTOPIA_DIR)/tmplib/include/  \
			  -I$(UTOPIA_DIR)/mxlib/include/  \


obj-m       := ms_vdec.o
ms_vdec-objs := $(OBJ_C)

export ARCH=arm

ifeq ($(CHIP),k6)
export CROSS_COMPILE=arm-none-linux-gnueabi-
KERNEL = /home/sylvia.nain/git/K6/tegra
else ifeq ($(CHIP),I2)
export CROSS_COMPILE=arm-linux-gnueabihf-
KERNEL = ../../../../../../project/kbuild/3.18.30/
endif

CC  = $(CROSS_COMPILE)gcc
EXTRA_CFLAGS  = $(CFG_DEFS)
EXTRA_CFLAGS  += $(API_INCS)

#files := $(foreach dir,$(PRIINC),$(wildcard $(dir)/*))

all:
	make modules -C $(KERNEL)  M=`pwd`

clean:
#	make modules clean -C $(KERNEL)  M=`pwd`
	@find ../ -name ".*.o.cmd" -exec rm -rf {} \;
	@find ../ -name "*.o" -exec rm -rf {} \;

#test:
#	@echo $(files)

test:
	@echo $(API_INCS)