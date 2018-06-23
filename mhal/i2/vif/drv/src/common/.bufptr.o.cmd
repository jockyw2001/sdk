cmd_/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/src/common/bufptr.o := arm-linux-gnueabihf-gcc -Wp,-MD,/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/src/common/.bufptr.o.d  -nostdinc -isystem /opt/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux/bin/../lib/gcc/arm-linux-gnueabihf/4.8.3/include -I./arch/arm/include -Iarch/arm/include/generated  -Iinclude -I./arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-dwarf2-cfi-asm -fno-ipa-sra -mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -fno-delete-null-pointer-checks -Werror -Os -Wno-maybe-uninitialized --param=allow-store-data-races=0 -Wframe-larger-than=4096 -fstack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -g -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -DCC_HAVE_ASM_GOTO -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/./inc -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/./pub -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/../../cmdq_service/drv/pub -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/../../../include/common -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/../hal/infinity2/pub -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/../hal/infinity2/inc -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/include/vif -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/include/common -I/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/include/cmdq_service -Iinclude/mstar -Idrivers/mstar/include -DMHAL_MLOAD=1 -DMHAL_AIO=1 -DMHAL_RGN=1 -DMHAL_VDEC=1 -DMHAL_MFE=1 -DMHAL_VPE=1 -DMHAL_WARP=1 -DMHAL_JPE=1 -DMHAL_NJPD=1 -DMHAL_VIF=1 -DMHAL_MHE=1 -DMHAL_DISP=1 -DMHAL_DIVP=1 -DMHAL_CMDQ_SERVICE=1 -DMHAL_PANEL=1 -DMHAL_RAW_DRV=0 -DMHAL_SIMPLE=0 -DMHAL_NEW_DRV=0 -I/home/wenshuai.xi/SI/i2_alkaid/project/../sdk/mhal/include/utopia -include utopia_macros.h    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(bufptr)"  -D"KBUILD_MODNAME=KBUILD_STR(bufptr)" -c -o /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/src/common/bufptr.o /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/src/common/bufptr.c

source_/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/src/common/bufptr.o := /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/src/common/bufptr.c

deps_/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/src/common/bufptr.o := \
  /home/wenshuai.xi/SI/i2_alkaid/project/../sdk/mhal/include/utopia/utopia_macros.h \
    $(wildcard include/config/utopia/framework/kernel/driver.h) \
    $(wildcard include/config/utopia/framework/kernel/driver/32bit.h) \
    $(wildcard include/config/utopiaxp/patch.h) \
  drivers/mstar/include/cam_os_wrapper.h \
  drivers/mstar/include/cam_os_util.h \
  drivers/mstar/include/cam_os_util_list.h \
  drivers/mstar/include/cam_os_util_bug.h \
  drivers/mstar/include/cam_os_util_hash.h \
  drivers/mstar/include/cam_os_util_bitmap.h \
  drivers/mstar/include/cam_os_util_ioctl.h \
  include/linux/kernel.h \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/panic/timeout.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  /opt/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux/lib/gcc/arm-linux-gnueabihf/4.8.3/include/stdarg.h \
  include/linux/linkage.h \
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
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
  arch/arm/include/asm/linkage.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
  include/linux/bitops.h \
  arch/arm/include/asm/bitops.h \
    $(wildcard include/config/smp.h) \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  include/linux/typecheck.h \
  arch/arm/include/asm/irqflags.h \
    $(wildcard include/config/cpu/v7m.h) \
  arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/arm/thumb.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  arch/arm/include/uapi/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
  arch/arm/include/asm/hwcap.h \
  arch/arm/include/uapi/asm/hwcap.h \
  arch/arm/include/asm/barrier.h \
    $(wildcard include/config/cpu/32v6k.h) \
    $(wildcard include/config/cpu/xsc3.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/arch/has/barriers.h) \
    $(wildcard include/config/arm/dma/mem/bufferable.h) \
  arch/arm/include/asm/outercache.h \
    $(wildcard include/config/outer/cache/sync.h) \
    $(wildcard include/config/ms/l2x0/patch.h) \
    $(wildcard include/config/outer/cache.h) \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/arch_hweight.h \
  include/asm-generic/bitops/const_hweight.h \
  include/asm-generic/bitops/lock.h \
  include/asm-generic/bitops/le.h \
  arch/arm/include/uapi/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/uapi/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  include/uapi/linux/swab.h \
  arch/arm/include/asm/swab.h \
  arch/arm/include/uapi/asm/swab.h \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/ext2-atomic-setbit.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/printk.h \
    $(wildcard include/config/message/loglevel/default.h) \
    $(wildcard include/config/early/printk.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
  include/linux/init.h \
    $(wildcard include/config/broken/rodata.h) \
    $(wildcard include/config/lto.h) \
  include/linux/kern_levels.h \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/uapi/linux/kernel.h \
  include/uapi/linux/sysinfo.h \
  arch/arm/include/asm/cache.h \
    $(wildcard include/config/arm/l1/cache/shift.h) \
    $(wildcard include/config/aeabi.h) \
  include/linux/dynamic_debug.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  include/uapi/linux/string.h \
  arch/arm/include/asm/string.h \
  include/linux/errno.h \
  include/uapi/linux/errno.h \
  arch/arm/include/generated/asm/errno.h \
  include/uapi/asm-generic/errno.h \
  include/uapi/asm-generic/errno-base.h \
  arch/arm/include/asm/div64.h \
  arch/arm/include/asm/compiler.h \
  arch/arm/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
    $(wildcard include/config/arm/lpae.h) \
  arch/arm/include/asm/opcodes.h \
    $(wildcard include/config/cpu/endian/be32.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  /home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/./pub/bufptr.h \

/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/src/common/bufptr.o: $(deps_/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/src/common/bufptr.o)

$(deps_/home/wenshuai.xi/SI/i2_alkaid/sdk/mhal/i2/vif/drv/src/common/bufptr.o):
