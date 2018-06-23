include clear-config.mk
API_FILE:=common.c
KAPI_DISABLE:=$(API_FILE)
UINTERNAL_FILES:=mi_os_posix.c
WRAPPER_FILE:=device.c
IMPL_FILES:=mi_os_linux.c
DEP_HAL:=common cmdq_service
include add-config.mk
