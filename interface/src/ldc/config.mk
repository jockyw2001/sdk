include clear-config.mk
#KAPI_DISABLE:=TRUE
DEP_MODULE:=common sys gfx
DEP_HAL_INC:=common warp
DEP_HAL:=ldc
API_FILE:=ldc_api.c
WRAPPER_FILE:=ldc_ioctl.c
LIBS:=mi_sys
IMPL_FILES:= mi_ldc_impl.c
include add-config.mk
