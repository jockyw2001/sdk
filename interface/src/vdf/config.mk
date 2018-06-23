include clear-config.mk
#KAPI_DISABLE:=TRUE
MACROS:=CAM_OS_LINUX_USER
DEP_MODULE:=common sys shadow md od
API_FILE:=vdf_api.c
WRAPPER_FILE:=
IMPL_FILES:=
include add-config.mk
