include clear-config.mk
DEP_MODULE:=common sys gfx
DEP_HAL_INC:=common
DEP_HAL:=vif
API_FILE:=vif_api.c
WRAPPER_FILE:=vif_ioctl.c
IMPL_FILES:= mi_vif_impl.c
include add-config.mk
