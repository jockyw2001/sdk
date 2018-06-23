include clear-config.mk
DEP_MODULE:common sys
CFLAGS:=-O0
SRCS:=hdmi.c
LIBS:=mi_hdmi
include add-config.mk
