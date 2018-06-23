include clear-config.mk
DEP_MODULE:=common sys
DEP_EXT_INC:=aio
DEP_HAL:=aio common
#UINTERNAL_FILES:=ao_api.c
API_FILE:=ao_api.c
WRAPPER_FILE:=ao_ioctl.c
#IMPL_FILES:= mi_ao_impl.c fake/mhal_audio.c  fake/mi_sys_fake.c
#IMPL_FILES:= mi_ao_impl.c fake/mhal_audio.c
IMPL_FILES:= mi_ao_impl.c
include add-config.mk
