include clear-config.mk
COMPILE_TARGET:=bin
SRCS:=../mi_disp_tc_common.c  disp_tc006.c
LIBS:= mi_sys mi_disp mi_hdmi mi_vpe
include add-config.mk
