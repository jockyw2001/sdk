#-------------------------------------------------------------------------------
#   Description of some variables owned by the library
#-------------------------------------------------------------------------------
# Library module (lib) or Binary module (bin)
PROCESS = lib
PATH_C +=\
    $(PATH_mfe_hal)/src

PATH_H +=\
    $(PATH_mfe_hal)/inc\
    $(PATH_mhve)/pub\
    $(PATH_cam_os_wrapper)/pub

#-------------------------------------------------------------------------------
#   List of source files of the library or executable to generate
#-------------------------------------------------------------------------------
SRC_C_LIST =\
    hal_mfe_ios.c\
    hal_h264_enc.c\
    hal_mfe_ops.c\
    hal_mfe_rqc.c\
    hal_msb2_rqc.c
