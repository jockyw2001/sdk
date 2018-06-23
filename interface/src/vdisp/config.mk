include clear-config.mk
DEP_MODULE:=common sys gfx
API_FILE:=vdisp_api.c
WRAPPER_FILE:=vdisp_ioctl.c
LIBS:=mi_sys
IMPL_FILES:= mi_vdisp_impl.c sub_buf_allocator.c vdisp_custom_allocator.c vdisp_customer_allocator_usermap.c
include add-config.mk
