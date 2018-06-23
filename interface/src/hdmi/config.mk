include clear-config.mk
DEP_MODULE:=common sys
API_FILE:=hdmi_api.c
WRAPPER_FILE:=hdmi_ioctl.c
IMPL_FILES:=mi_hdmi_impl.c
KAPI_DISABLE:=TRUE
include add-config.mk
