include clear-config.mk
CFLAGS:=-O0
SRCS:=mi_divp_test.c
LIBS:=mi_sys mi_vdec mi_divp mi_disp mi_vpe mi_hdmi
UINTERNAL_FILES = mi_divp_test.c
include add-config.mk