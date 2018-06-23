include clear-config.mk
#KAPI_DISABLE:=TRUE
DEP_MODULE:=common sys gfx
DEP_HAL_INC:=common
DEP_HAL:=warp
API_FILE:=warp_api.c
WRAPPER_FILE:=warp_ioctl.c
LIBS:=mi_sys
IMPL_FILES:= mi_warp_impl.c
include add-config.mk
