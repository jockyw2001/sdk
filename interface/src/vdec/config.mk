include clear-config.mk
DEP_MODULE:=common sys
API_FILE:=vdec_api.c
FRMING_SRC:=vdec_framing.c
UINTERNAL_FILES:=${FRMING_SRC}
WRAPPER_FILE:=vdec_ioctl.c
IMPL_FILES:=mi_vdec_impl.c
#IMPL_FILES:=mi_vdec_impl.c ${FRMING_SRC}
KAPI_DISABLE:=TRUE
include add-config.mk
