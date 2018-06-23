include clear-config.mk
COMPILE_TARGET:=bin
SRCS:=  ../mi_disp_tc_common.c disp_tc013.c
LIBS:= mi_sys mi_disp mi_hdmi mi_vpe mi_divp
CFLAGS+= -Wall
include add-config.mk
