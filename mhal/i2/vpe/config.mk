include raw-impl-config.mk
DEPS:=common cmdq_service mload vpe
#MACROS:=SCLOS_TYPE_LINUX_KERNEL SCLOS_TYPE_LINUX_KERNEL_I2 SCLOS_TYPE_LINUX_TESTCMDQ SCLOS_TYPE_LINUX_TEST USE_USBCAM
MACROS:=SCLOS_TYPE_LINUX_KERNEL SCLOS_TYPE_LINUX_KERNEL_I2 USE_USBCAM

DRV_PUB_INCS:=MI_HAL/pub/mhal_vpe.h
DRV_INC_PATH:=scl/inc/linux scl/inc/pq scl/pub/ scl/pub/linux MI_HAL/pub MI_HAL/inc ../hal/scl/inc/drv/linux ../../cmdq_service/drv/pub ../../../include/common isp/pub ../../vif/drv/pub isp/pub
DRV_SRCS:=scl/src/linux/drv_scl_os.c \
scl/src/linux/drv_scl_ioctl_parse.c \
scl/src/linux/drv_scl_module.c \
scl/src/ut/scl_ut.c \
scl/src/pq/drv_scl_pq.c \
MI_HAL/src/mhal_vpe.c \

HAL_PUB_PATH:=scl/inc/drv scl/inc/hal scl/inc/pq
HAL_INC_PATH:=scl/inc/drv scl/inc/drv/linux scl/inc/hal scl/inc/pq ../drv/scl/inc/linux ../drv/scl/inc/pq ../drv/scl/pub/ ../drv/scl/pub/linux ../../cmdq_service/drv/pub/ ../../../include/common ../../mload/drv/pub/
HAL_SRCS:=scl/src/drv/drv_scl_ctx_m.c \
scl/src/hal/hal_scl_dma.c \
scl/src/hal/hal_scl_hvsp.c \
scl/src/hal/hal_scl_vip.c \
scl/src/hal/hal_scl_irq.c \
scl/src/hal/hal_utility.c \
scl/src/pq/hal_scl_pq_qualitymap_main.c \
scl/src/drv/linux/drv_scl_irq.c \
scl/src/drv/drv_scl_dma.c \
scl/src/drv/drv_scl_hvsp.c \
scl/src/drv/drv_scl_vip.c \
scl/src/drv/drv_scl_dma_m.c \
scl/src/drv/drv_scl_hvsp_m.c \
scl/src/drv/drv_scl_vip_m.c \
scl/src/drv/drv_scl_dma_io_wrapper.c \
scl/src/drv/drv_scl_vip_io_wrapper.c \
scl/src/drv/drv_scl_m2m_io_wrapper.c \
scl/src/drv/drv_scl_hvsp_io_wrapper.c
include add-config.mk
