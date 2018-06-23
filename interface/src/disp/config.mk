include clear-config.mk
DEP_MODULE:=common sys gfx divp
DEP_HAL:=disp common
API_FILE:=disp_api.c
WRAPPER_FILE:=disp_ioctl.c
#IMPL_FILES:= mi_disp_impl.c fake/hal_disp.c  fake/mi_sys_fake.c
LIBS:=mi_sys mi_gfx mi_divp
# FPGA test
#IMPL_FILES:= mi_disp_impl.c fake/hal_disp.c
IMPL_FILES:= mi_disp_impl.c mi_disp_cus_allocator.c
# mhal_disp.c
#UINTERNAL_FILES:= disp_internal_api.c
include add-config.mk
