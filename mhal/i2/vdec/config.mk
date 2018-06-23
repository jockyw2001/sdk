include raw-impl-config.mk

DEPS:= vdec

DRV_PUB_INCS:=api/pub/apiVDEC_EX.h
DRV_INC_PATH:= api/vdec_v3  hvd_v3 mvd_v3 mjpeg_v3 api/pub api/inc
DRV_SRCS:= api/vdec_v3/apiVDEC_EX.c \
            hvd_v3/drvHVD_EX.c \
            hvd_v3/osalHVD_EX.c \
            hvd_v3/HVD_EX_Common.c \
            mvd_v3/drvMVD_EX.c \
            mvd_v3/osalMVD_EX.c \
            mvd_v3/mvd_cc_EX.c \
            mvd_v3/MVD_EX_Common.c \
            api/vdec_v3/apiMJPEG_EX.c \
            mjpeg_v3/drvMJPEG_EX.c \
            mjpeg_v3/VirtualhalMJPEG_EX.c \
            api/extrn/__aeabi_uldivmod.c \
            linux/drv_vdec_export.c

HAL_PUB_PATH:= inc hvd_v3 vpu_v3
HAL_INC_PATH:= hvd_v3 mvd_v3 vpu_v3
HAL_SRCS:=	hvd_v3/halHVD_EX.c \
            mvd_v3/halMVD_EX.c \
            vpu_v3/halVPU_EX.c \
			vpu_v3/ms_decompress.c \

# **********************************************
# define options
# **********************************************
MACROS := _REWRITE_UTOPIA_FOP
#MACROS += UFO_PUBLIC_HEADER_700
#MACROS += _USE_MDRV_VDEC_ _USE_MAPI_IF
#MACROS += MSOS_TYPE_LINUX_KERNEL

#MACROS += CHIP_I2_FPGA
MACROS += KERNEL_DRIVER_PATCH
#MACROS += _DISABLE_CLK_CTL

# **********************************************
# define option from Utopia build/Makefile
# **********************************************
#CHIP_NAME = $(shell echo $(CHIP) | tr a-z A-Z)
#MCU_TYPE = "arm_ca7"
#CPU_NAME = armca7-D

#MACROS +=  CHIP_K6
#MACROS +=  'CHIP_$(CHIP_NAME)'  'MS_C_STDLIB'
#MACROS +=  'MCU_ARM_CA7'
#MACROS +=  'DONT_USE_CMA'
#MACROS +=  'SUPPORT_EVD=1'
MACROS +=  SUPPORT_EVD
MACROS +=  'SUPPORT_NEW_MEM_LAYOUT'

# **********************************************
# define option for VDEC3
# **********************************************
#MACROS +=  VDEC3
#MACROS +=  VDEC3_FB
#MACROS +=  SUPPORT_MSVP9=0 SUPPORT_G2VP9=0

# **********************************************
# Option to disable some functionalities
# **********************************************
MACROS  +=  _ENABLE_VDEC_MVD
MACROS  +=  _DISABLE_MDRV_SYS
MACROS  +=  _DISABLE_VDEC_MBX
MACROS  +=  _DISABLE_GETLIBVER

#disable the following options
#MACROS  +=  _ENABLE_VDEC_SECUMODE
#MACROS  +=  _ENABLE_VDEC_MJPEG


include add-config.mk