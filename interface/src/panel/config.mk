include clear-config.mk
DEP_MODULE:=common sys
DEP_HAL:=panel common
API_FILE:=panel_api.c
WRAPPER_FILE:=panel_ioctl.c
IMPL_FILES:=mi_panel_impl.c
KAPI_DISABLE:=TRUE
include add-config.mk
