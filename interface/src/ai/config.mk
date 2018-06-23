include clear-config.mk
DEP_MODULE:=common sys
DEP_EXT_INC:=aio
DEP_HAL:=aio common
API_FILE:=ai_api.c
WRAPPER_FILE:=ai_ioctl.c
#IMPL_FILES:= mi_ai_impl.c fake/mhal_audio.c  fake/mi_sys_fake.c
#IMPL_FILES:= mi_ai_impl.c fake/mhal_audio.c
IMPL_FILES:= mi_ai_impl.c
include add-config.mk
