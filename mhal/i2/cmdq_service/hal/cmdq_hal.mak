#-------------------------------------------------------------------------------
#	Description of some variables owned by the library
#-------------------------------------------------------------------------------
# Library module (lib) or Binary module (bin)
PROCESS = lib
PATH_C += \
          $(PATH_cmdq_hal)/src

PATH_H += $(PATH_cmdq_hal)/pub\
          $(PATH_cmdq)/inc/rtk\
          $(PATH_cam_os_wrapper)/pub\
          $(PATH_cam_os_wrapper)/inc

SRC_C_LIST = \
      hal_cmdq.c
