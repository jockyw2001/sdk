include clear-config.mk
DEP_MODULE:=common sys
DEP_HAL:=exp
API_FILE:=uac_api.c
WRAPPER_FILE:=uac_ioctl.c
IMPL_FILES:=mi_uac_impl.c mi_alsa.c
include add-config.mk
