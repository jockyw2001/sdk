include clear-config.mk
DEP_MODULE:=common sys
DEP_HAL:=venc cmdq_service mfe mhe jpe common
API_FILE:=venc_api.c
WRAPPER_FILE:=venc_ioctl.c
#IMPL_FILES:=mi_venc_impl.c
#IMPL_FILES:=mi_venc_impl.c ../../hal/simulation/venc/src/mhal_venc_dummy.c
IMPL_FILES:=mi_venc_impl.c mi_venc_linux.c src/mhal_venc_dummy.c src/file_access.c src/venc_util.c
include add-config.mk
