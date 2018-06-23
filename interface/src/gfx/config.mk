include clear-config.mk
DEP_MODULE:=common sys
API_FILE:=gfx_api.c
WRAPPER_FILE:=gfx_ioctl.c
IMPL_FILES:=mi_gfx_impl.c
include add-config.mk
