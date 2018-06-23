include clear-config.mk
DEP_MODULE:=common sys gfx rgn vif
API_FILE:= sd_api.c
WRAPPER_FILE:= sd_ioctl.c
DEP_HAL:=vpe common cmdq_service
IMPL_FILES:=  mi_sd_impl.c
include add-config.mk
