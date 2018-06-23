include clear-config.mk
CFLAGS:=-O0
SRCS:=mi_ai_test.c
LIBS:=mi_sys mi_ai SRC_LINUX APC_LINUX AEC_LINUX g711 g726
include add-config.mk