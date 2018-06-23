
#------------------------------------------------------------------------------
#	Description of some variables owned by the library
#------------------------------------------------------------------------------
# Library module (lib) or Binary module (bin)
PROCESS		= lib

#JPE_PLATFORM = linux
JPE_PLATFORM = rtk

PATH_C +=\
    $(PATH_jpe)/src/common\
    $(PATH_jpe)/src/$(JPE_PLATFORM)\
    $(PATH_jpe)/../test/user_ut/common\
    $(PATH_jpe)/../test/user_ut/$(JPE_PLATFORM)\
    $(PATH_jpe)/../test/kernel_ut/

PATH_H +=\
    $(PATH_jpe)/inc\
    $(PATH_jpe_hal)/inc\
    $(PATH_jpe_hal)/pub\
    $(PATH_cam_os_wrapper)/pub\
    $(PATH_jpe)/pub\
    $(PATH_jpe)/../test/user_ut/common\
    $(PATH_jpe)/../test/user_ut/$(JPE_PLATFORM)\
    $(PATH_jpe)/../test/kernel_ut/

#------------------------------------------------------------------------------
#	List of source files of the library or executable to generate
#------------------------------------------------------------------------------

SRC_C_LIST = \
    drv_jpe_ctx.c\
    drv_jpe_dev.c\
    drv_jpe_module.c\
    drv_jpe_clk.c\
    drv_jpe_enc.c\
    jpegenc_marker.c\
    jpe_test_i_sw.c\
    util_pattern.c\
    md5.c\
    mhal_jpe.c\
    jpe_ut_rtk.c