include clear-config.mk
CFLAGS:=-O0
SRCS:=vdisp_test_common.c vdisp_test.c
LIBS:=mi_sys mi_vdisp mi_vpe
include add-config.mk