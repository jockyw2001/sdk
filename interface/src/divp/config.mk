include clear-config.mk
DEP_MODULE:=common sys rgn gfx
DEP_HAL:=divp cmdq_service common
API_FILE:=divp_api.c
DEP_EXT_INC:= vdec
WRAPPER_FILE:=divp_ioctl.c
IMPL_FILES:=mi_divp_impl.c
include add-config.mk
