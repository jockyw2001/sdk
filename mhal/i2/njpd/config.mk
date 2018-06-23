include hal-impl-config.mk
MACROS:=
DEPS:= NJPE_EX

DRV_PUB_INCS:= apiJPEG.h \
		       apiJPEG_v2.h

DRV_INC_PATH:= drvNJPD.h \
			   jpeg_memory.h \
			   njpeg_def.h \
			   osalNJPD.h

DRV_SRCS:=  apiJPEG.c \
			jpeg_memory.c \
			osalNJPD.c \
		    drvNJPD.c \
		    linux/drv_njpd_export.c

HAL_INC_PATH:= halNJPD.h \
               regNJPD.h

HAL_SRCS:= halNJPD.c

include add-config.mk
