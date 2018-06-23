include clear-config.mk
DEP_MODULE:=common sys gfx
DEP_HAL:=exp
API_FILE:=foo_api.c
WRAPPER_FILE:=foo_ioctl.c
IMPL_FILES:=foo.c vif.c vpe.c vdec.c disp.c divp.c ai.c ao.c venc.c
include add-config.mk
