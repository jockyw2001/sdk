include clear-config.mk
DEP_MODULE:=common sys
DEP_HAL:=exp
API_FILE:=shadow_api.c
WRAPPER_FILE:=shadow_ioctl.c
IMPL_FILES:=mi_shadow_impl.c
include add-config.mk
