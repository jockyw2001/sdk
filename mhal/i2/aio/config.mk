include hal-impl-config.mk
DEPS:=common

DRV_PUB_INCS:=mhal_audio.h mhal_audio_datatype.h
DRV_INC_PATH:=./
DRV_SRCS:=common/mhal_audio.c linux/drv_audio.c

HAL_PUB_PATH:=./
HAL_INC_PATH:=
HAL_SRCS:=hal_audio.c

include add-config.mk
