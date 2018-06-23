#------------------------------------------------------------------------------
# Description of some variables owned by the library
#------------------------------------------------------------------------------
# Library module (lib) or Binary module (bin)
PROCESS   = lib
#------------------------------------------------------------------------------
# List of source files of the library or executable to generate
#------------------------------------------------------------------------------
PATH_C += \
          $(PATH_cmdq)/src\
          $(PATH_cmdq)/src/rtk

PATH_H += $(PATH_cmdq_hal)/pub\
          $(PATH_cmdq)/inc\
          $(PATH_cmdq)/inc/rtk\
          $(PATH_cmdq)/pub\
          $(PATH_camdrv)/include\
          $(PATH_cam_os_wrapper)/pub\
          $(PATH_cam_os_wrapper)/inc

SRC_C_LIST = \
      drv_cmdq.c \
      mhal_cmdq.c \
      cmdq_proc.c \
      drv_cmdq_os.c \
      drv_cmdq_irq.c \
      cmdq_test.c
