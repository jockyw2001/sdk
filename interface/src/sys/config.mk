include clear-config.mk
DEP_MODULE:=common
DEP_HAL:=common cmdq_service
API_FILE:=sys_api.c
WRAPPER_FILE:=sys_ioctl.c
UINTERNAL_FILES:=debug_level.c
IMPL_FILES:=mi_sys_internal.c mi_sys_impl.c mi_sys_buf_mgr.c mi_sys_mma_heap_impl.c mi_sys_chunk_impl.c mi_sys_meta_impl.c mi_sys_vb_pool_impl.c mi_sys_mma_miu_protect_impl.c drv_proc_ext.c drv_proc_ext_k.c mi_sys_log_impl.c mi_sys_trace.c mi_syscfg_config.c mi_syscfg_mmap.c linux.c
include add-config.mk
