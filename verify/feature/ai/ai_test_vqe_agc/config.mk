include clear-config.mk
COMPILE_TARGET:=bin
CFLAGS:=-O0
SRCS:=ai_test_vqe_agc.c
LIBS:=mi_sys mi_ai SRC_LINUX APC_LINUX AEC_LINUX g711 g726
include add-config.mk