include clear-config.mk
CFLAGS:=-O0
SRCS:=mi_panel_test.c mi_panel_test_tb.c
LIBS:=mi_sys mi_panel mi_disp
UINTERNAL_FILES = mi_panel_test.c
include add-config.mk
