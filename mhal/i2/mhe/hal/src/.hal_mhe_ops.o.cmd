cmd_/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/src/hal_mhe_ops.o := arm-linux-gnueabihf-gcc -Wp,-MD,/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/src/.hal_mhe_ops.o.d  -nostdinc -isystem /opt/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux/bin/../lib/gcc/arm-linux-gnueabihf/4.8.3/include -I./arch/arm/include -Iarch/arm/include/generated  -Iinclude -I./arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-dwarf2-cfi-asm -fno-ipa-sra -mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -fno-delete-null-pointer-checks -Werror -Os -Wno-maybe-uninitialized --param=allow-store-data-races=0 -Wframe-larger-than=4096 -fstack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -g -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -DCC_HAVE_ASM_GOTO -DENABLE_DUMP_REG=1 -DSUPPORT_CMDQ_SERVICE=1 -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/inc -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/pub -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/include/common -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/include/venc -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/include/cmdq_service -Iinclude/mstar -Idrivers/mstar/include -DMHAL_MLOAD=1 -DMHAL_AIO=1 -DMHAL_RGN=1 -DMHAL_VDEC=1 -DMHAL_MFE=1 -DMHAL_VPE=1 -DMHAL_WARP=1 -DMHAL_JPE=1 -DMHAL_NJPD=1 -DMHAL_VIF=1 -DMHAL_MHE=1 -DMHAL_DISP=1 -DMHAL_DIVP=1 -DMHAL_CMDQ_SERVICE=1 -DMHAL_PANEL=1 -DMHAL_RAW_DRV=0 -DMHAL_SIMPLE=0 -DMHAL_NEW_DRV=0 -I/home/wenshuai.xi/SI/i2_alkaid/project/../sdk/mhal/include/utopia -include utopia_macros.h    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(hal_mhe_ops)"  -D"KBUILD_MODNAME=KBUILD_STR(hal_mhe_ops)" -c -o /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/src/hal_mhe_ops.o /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/src/hal_mhe_ops.c

source_/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/src/hal_mhe_ops.o := /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/src/hal_mhe_ops.c

deps_/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/src/hal_mhe_ops.o := \
  /home/wenshuai.xi/SI/i2_alkaid/project/../sdk/mhal/include/utopia/utopia_macros.h \
    $(wildcard include/config/utopia/framework/kernel/driver.h) \
    $(wildcard include/config/utopia/framework/kernel/driver/32bit.h) \
    $(wildcard include/config/utopiaxp/patch.h) \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/inc/hal_mhe_ops.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/pub/mhve_ops.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/pub/hal_mhe_def.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/64bit.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  include/uapi/linux/types.h \
  arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  include/uapi/asm-generic/int-ll64.h \
  arch/arm/include/generated/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/uapi/asm-generic/bitsperlong.h \
  include/uapi/linux/posix_types.h \
  include/linux/stddef.h \
  include/uapi/linux/stddef.h \
  arch/arm/include/uapi/asm/posix_types.h \
  include/uapi/asm-generic/posix_types.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
  /opt/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux/lib/gcc/arm-linux-gnueabihf/4.8.3/include/stdarg.h \
  include/uapi/linux/string.h \
  arch/arm/include/asm/string.h \
  drivers/mstar/include/cam_os_wrapper.h \
  drivers/mstar/include/cam_os_util.h \
  drivers/mstar/include/cam_os_util_list.h \
  drivers/mstar/include/cam_os_util_bug.h \
  drivers/mstar/include/cam_os_util_hash.h \
  drivers/mstar/include/cam_os_util_bitmap.h \
  drivers/mstar/include/cam_os_util_ioctl.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/inc/hal_h265_enc.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/inc/hal_mhe_reg.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/pub/mhve_ios.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/pub/mhve_pmbr_cfg.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/inc/hal_mhe_ios.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/inc/hal_mhe_rqc.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/pub/mhve_rqct_ops.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/pub/mhve_rqct_cfg.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/inc/hal_mhe_pmbr.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/pub/mhve_pmbr_ops.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/pub/mhve_ios.h \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/pub/mhve_pmbr_cfg.h \

/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/src/hal_mhe_ops.o: $(deps_/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/src/hal_mhe_ops.o)

$(deps_/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/mhe/hal/src/hal_mhe_ops.o):
