include clear-config.mk
COMPILE_TARGET:=bin
CFLAGS:=-O0
SRCS:=disp.c
LIBS:=mi_sys mi_disp
include add-config.mk
