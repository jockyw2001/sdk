include clear-config.mk
DEP_MODULE:=common sys
DEP_EXT_INC:=bar_dep
UINTERNAL_FILES:=bar_user.c
API_FILE:=bar_api.c
WRAPPER_FILE:=bar_ioctl.c
IMPL_FILE$(interface_foo):=bar_impl.c
include add-config.mk
