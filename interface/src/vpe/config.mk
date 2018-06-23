include clear-config.mk
DEP_MODULE:=common sys gfx rgn
API_FILE:=vpe_api.c
WRAPPER_FILE:=vpe_ioctl.c
DEP_HAL:=vpe common cmdq_service
DEP_EXT_INC:=vif
# real use:
#IMPL_FILES:= mi_vpe_impl_3dnr_update.c  mi_vpe_impl_roi.c mi_vpe_impl.c
# verify/module/vpe/ use:
#IMPL_FILES:= mi_vpe_impl_3dnr_update.c  mi_vpe_impl_roi.c mi_vpe_impl.c fake/mhal_vpe.c fake/mi_sys_fake.c fake/cmdq_service_fake.c
# verifty/feature/vpe use:
ifeq ($(CHIP),k6l)
IMPL_FILES:= mi_vpe_impl_3dnr_update.c  mi_vpe_impl_roi.c mi_vpe_impl.c fake/mhal_vpe_withge.c
else ifeq ($(CHIP),k6)
IMPL_FILES:= mi_vpe_impl_3dnr_update.c  mi_vpe_impl_roi.c mi_vpe_impl.c fake/mhal_vpe_withge.c
else
IMPL_FILES:= mi_vpe_impl_3dnr_update.c  mi_vpe_impl_roi.c mi_vpe_impl.c
endif
include add-config.mk
