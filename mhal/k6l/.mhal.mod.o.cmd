cmd_/home/wenshuai.xi/SI/alkaid/sdk/mhal/k6l/mhal.mod.o := arm-buildroot-linux-uclibcgnueabi-gcc -Wp,-MD,/home/wenshuai.xi/SI/alkaid/sdk/mhal/k6l/.mhal.mod.o.d  -nostdinc -isystem /opt/arm_4.7.2_with_fortify/bin/../lib/gcc/arm-buildroot-linux-uclibcgnueabi/4.7.2/include -I/home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/uapi -Iinclude/generated/uapi -include /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/include -Idrivers/mstar2 -Iarch/arm/arm-boards/generic/include/ -Iarch/arm/arm-boards/../../../../mstar2/hal/k6lite/cpu//include -Iarch/arm/arm-boards/plat-mstar//include -Iarch/arm/arm-boards/../../../../mstar2/hal/k6lite/cpu/ -Iarch/arm/arm-boards/plat-mstar/ -Idrivers/mstar -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -fno-peephole2 -O2 -g -fconserve-stack -fno-dwarf2-cfi-asm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -fstack-protector -fno-conserve-stack -mabi=aapcs-linux -mno-thumb-interwork -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -Wframe-larger-than=1024 -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(mhal.mod)"  -D"KBUILD_MODNAME=KBUILD_STR(mhal)" -DMODULE  -c -o /home/wenshuai.xi/SI/alkaid/sdk/mhal/k6l/mhal.mod.o /home/wenshuai.xi/SI/alkaid/sdk/mhal/k6l/mhal.mod.c

source_/home/wenshuai.xi/SI/alkaid/sdk/mhal/k6l/mhal.mod.o := /home/wenshuai.xi/SI/alkaid/sdk/mhal/k6l/mhal.mod.c

deps_/home/wenshuai.xi/SI/alkaid/sdk/mhal/k6l/mhal.mod.o := \
  include/linux/module.h \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/module/sig.h) \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/tracepoints.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/event/tracing.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/constructors.h) \
    $(wildcard include/config/debug/set/module/ronx.h) \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/types.h \
    $(wildcard include/config/mp/debug/tool/changelist.h) \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  include/uapi/linux/types.h \
  arch/arm/include/generated/asm/types.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/uapi/asm-generic/types.h \
  include/asm-generic/int-ll64.h \
  include/uapi/asm-generic/int-ll64.h \
  arch/arm/include/generated/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/uapi/asm-generic/bitsperlong.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/uapi/linux/posix_types.h \
  include/linux/stddef.h \
  include/uapi/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/uapi/asm/posix_types.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/uapi/asm-generic/posix_types.h \
  include/linux/poison.h \
    $(wildcard include/config/illegal/pointer/value.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/uapi/linux/const.h \
  include/linux/stat.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/uapi/asm/stat.h \
  include/uapi/linux/stat.h \
  include/linux/time.h \
    $(wildcard include/config/arch/uses/gettimeoffset.h) \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/ring/buffer.h) \
  /opt/arm_4.7.2_with_fortify/bin/../lib/gcc/arm-buildroot-linux-uclibcgnueabi/4.7.2/include/stdarg.h \
  include/linux/linkage.h \
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modversions.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/linkage.h \
  include/linux/bitops.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/bitops.h \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
    $(wildcard include/config/kdebugd/ftrace.h) \
  include/linux/typecheck.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/irqflags.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/arm/thumb.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/uapi/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/hwcap.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/uapi/asm/hwcap.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/uapi/mstar/mpatch_macro.h \
    $(wildcard include/config/mp/platform/arch/general.h) \
    $(wildcard include/config/mp/platform/arch/general/debug.h) \
    $(wildcard include/config/mp/platform/arm.h) \
    $(wildcard include/config/mp/platform/arm/debug.h) \
    $(wildcard include/config/mp/platform/mips.h) \
    $(wildcard include/config/mp/platform/mips/debug.h) \
    $(wildcard include/config/mp/platform/pm.h) \
    $(wildcard include/config/mp/platform/pm/debug.h) \
    $(wildcard include/config/mp/platform/arm/errata/775420.h) \
    $(wildcard include/config/mp/platform/arm/errata/775420/debug.h) \
    $(wildcard include/config/mp/platform/arm/errata/835769.h) \
    $(wildcard include/config/mp/platform/arm/errata/835769/debug.h) \
    $(wildcard include/config/mp/platform/arm/64bit/poarting.h) \
    $(wildcard include/config/mp/platform/arm/64bit/poarting/debug.h) \
    $(wildcard include/config/mp/platform/arm/64bit/bootargs/nodtb.h) \
    $(wildcard include/config/mp/platform/arm/64bit/bootargs/nodtb/debug.h) \
    $(wildcard include/config/mp/platform/mstar/legancy/intr.h) \
    $(wildcard include/config/mp/platform/mstar/legancy/intr/debug.h) \
    $(wildcard include/config/mp/platform/sw/patch/l2/sram/rma.h) \
    $(wildcard include/config/mp/platform/sw/patch/l2/sram/rma/debug.h) \
    $(wildcard include/config/mp/platform/mips74k/timer.h) \
    $(wildcard include/config/mp/platform/mips74k/timer/debug.h) \
    $(wildcard include/config/mp/platform/arm/mstar/etm.h) \
    $(wildcard include/config/mp/platform/arm/mstar/etm/debug.h) \
    $(wildcard include/config/mp/platform/int/1/to/1/spi.h) \
    $(wildcard include/config/mp/platform/int/1/to/1/spi/debug.h) \
    $(wildcard include/config/mp/platform/arm/pmu.h) \
    $(wildcard include/config/mp/platform/arm/pmu/debug.h) \
    $(wildcard include/config/mp/platform/lzma/bin/compressed.h) \
    $(wildcard include/config/mp/platform/lzma/bin/compressed/debug.h) \
    $(wildcard include/config/mp/platform/cpu/setting.h) \
    $(wildcard include/config/mp/platform/cpu/setting/debug.h) \
    $(wildcard include/config/mp/platform/t/sensor/observation.h) \
    $(wildcard include/config/mp/platform/t/sensor/observation/debug.h) \
    $(wildcard include/config/mp/platform/fixme.h) \
    $(wildcard include/config/mp/platform/fixme/debug.h) \
    $(wildcard include/config/mp/platform/fiq/irq/hyp.h) \
    $(wildcard include/config/mp/platform/fiq/irq/hyp/debug.h) \
    $(wildcard include/config/mp/platform/utopia2/interrupt.h) \
    $(wildcard include/config/mp/platform/utopia2/interrupt/debug.h) \
    $(wildcard include/config/mp/platform/utopia2k/export/symbol.h) \
    $(wildcard include/config/mp/platform/utopia2k/export/symbol/debug.h) \
    $(wildcard include/config/mp/platform/arm/memory/hotplug.h) \
    $(wildcard include/config/mp/platform/arm/memory/hotplug/debug.h) \
    $(wildcard include/config/mp/platform/arm64/dtb/irqnum/check/off.h) \
    $(wildcard include/config/mp/platform/arm64/dtb/irqnum/check/off/debug.h) \
    $(wildcard include/config/mp/platform/frc/mapping.h) \
    $(wildcard include/config/mp/platform/frc/mapping/debug.h) \
    $(wildcard include/config/mp/platform/gic/set/multiple/cpus.h) \
    $(wildcard include/config/mp/platform/gic/set/multiple/cpus/debug.h) \
    $(wildcard include/config/mp/platform/dvfs/get/err/state/from/cpuhotplg/disable.h) \
    $(wildcard include/config/mp/platform/dvfs/get/err/state/from/cpuhotplg/disable/debug.h) \
    $(wildcard include/config/mp/platform/pipe/flush/double/check.h) \
    $(wildcard include/config/mp/platform/pipe/flush/double/check/debug.h) \
    $(wildcard include/config/mp/platform/module/compile/conf/check.h) \
    $(wildcard include/config/mp/platform/module/compile/conf/check/debug.h) \
    $(wildcard include/config/mp/nand/ubi.h) \
    $(wildcard include/config/mp/nand/ubi/debug.h) \
    $(wildcard include/config/mp/nand/mtd.h) \
    $(wildcard include/config/mp/nand/mtd/debug.h) \
    $(wildcard include/config/mp/nand/ubifs.h) \
    $(wildcard include/config/mp/nand/ubifs/debug.h) \
    $(wildcard include/config/mp/nand/spansion.h) \
    $(wildcard include/config/mp/nand/spansion/debug.h) \
    $(wildcard include/config/mp/nand/bbt.h) \
    $(wildcard include/config/mp/nand/bbt/debug.h) \
    $(wildcard include/config/mp/nand/bbt/size.h) \
    $(wildcard include/config/mp/nand/bbt/size/debug.h) \
    $(wildcard include/config/mp/usb/mstar.h) \
    $(wildcard include/config/mp/usb/mstar/debug.h) \
    $(wildcard include/config/mp/usb/str/patch.h) \
    $(wildcard include/config/mp/usb/str/patch/debug.h) \
    $(wildcard include/config/mp/emmc/trim.h) \
    $(wildcard include/config/mp/emmc/trim/debug.h) \
    $(wildcard include/config/mp/checkpt/boot.h) \
    $(wildcard include/config/mp/checkpt/boot/debug.h) \
    $(wildcard include/config/mp/compiler/error.h) \
    $(wildcard include/config/mp/compiler/error/debug.h) \
    $(wildcard include/config/mp/android/dummy/mstar/rtc.h) \
    $(wildcard include/config/mp/android/dummy/mstar/rtc/debug.h) \
    $(wildcard include/config/mp/android/mstar/hardcode/lpj.h) \
    $(wildcard include/config/mp/android/mstar/hardcode/lpj/debug.h) \
    $(wildcard include/config/mp/android/mstar/rc/map/define.h) \
    $(wildcard include/config/mp/android/mstar/rc/map/define/debug.h) \
    $(wildcard include/config/mp/android/mstar/adjust/low/mem/killer/policy.h) \
    $(wildcard include/config/mp/android/mstar/adjust/low/mem/killer/policy/debug.h) \
    $(wildcard include/config/mp/android/mstar/change/irq/file/permission.h) \
    $(wildcard include/config/mp/android/mstar/change/irq/file/permission/debug.h) \
    $(wildcard include/config/mp/debug/tool/coredump.h) \
    $(wildcard include/config/mp/debug/tool/coredump/debug.h) \
    $(wildcard include/config/mp/debug/tool/kdebug.h) \
    $(wildcard include/config/mp/debug/tool/kdebug/debug.h) \
    $(wildcard include/config/mp/debug/tool/coredump/path/bootargs/only.h) \
    $(wildcard include/config/mp/debug/tool/coredump/path/bootargs/only/debug.h) \
    $(wildcard include/config/mp/debug/tool/coredump/usb/default.h) \
    $(wildcard include/config/mp/debug/tool/coredump/usb/default/debug.h) \
    $(wildcard include/config/mp/debug/tool/coredump/path/backup.h) \
    $(wildcard include/config/mp/debug/tool/coredump/path/backup/debug.h) \
    $(wildcard include/config/mp/debug/tool/oprofile.h) \
    $(wildcard include/config/mp/debug/tool/oprofile/debug.h) \
    $(wildcard include/config/mp/debug/tool/pmu/timer/mode.h) \
    $(wildcard include/config/mp/debug/tool/pmu/timer/mode/debug.h) \
    $(wildcard include/config/mp/debug/tool/vm/used/size/check.h) \
    $(wildcard include/config/mp/debug/tool/vm/used/size/check/debug.h) \
    $(wildcard include/config/mp/debug/tool/coredump/build/in/usb.h) \
    $(wildcard include/config/mp/debug/tool/coredump/build/in/usb/debug.h) \
    $(wildcard include/config/mp/debug/tool/coredump/without/compress.h) \
    $(wildcard include/config/mp/debug/tool/coredump/without/compress/debug.h) \
    $(wildcard include/config/mp/debug/tool/changelist/debug.h) \
    $(wildcard include/config/mp/debug/tool/memory/usage/monitor.h) \
    $(wildcard include/config/mp/debug/tool/memory/usage/monitor/debug.h) \
    $(wildcard include/config/mp/debug/tool/ramlog.h) \
    $(wildcard include/config/mp/debug/tool/ramlog/debug.h) \
    $(wildcard include/config/mp/debug/tool/memory/usage/trace.h) \
    $(wildcard include/config/mp/debug/tool/memory/usage/trace/debug.h) \
    $(wildcard include/config/mp/debug/tool/thread/create/monitor.h) \
    $(wildcard include/config/mp/debug/tool/thread/create/monitor/debug.h) \
    $(wildcard include/config/mp/debug/tool/coredump/keep/kicking/wdt.h) \
    $(wildcard include/config/mp/debug/tool/coredump/keep/kicking/wdt/debug.h) \
    $(wildcard include/config/mp/debug/tool/dump/stack/each/cpu.h) \
    $(wildcard include/config/mp/debug/tool/dump/stack/each/cpu/debug.h) \
    $(wildcard include/config/mp/mips/l2/cache.h) \
    $(wildcard include/config/mp/mips/l2/cache/debug.h) \
    $(wildcard include/config/mp/smp/startup.h) \
    $(wildcard include/config/mp/smp/startup/debug.h) \
    $(wildcard include/config/mp/uart/serial/8250/riu/base.h) \
    $(wildcard include/config/mp/uart/serial/8250/riu/base/debug.h) \
    $(wildcard include/config/mp/ntfs3g/wrap.h) \
    $(wildcard include/config/mp/ntfs3g/wrap/debug.h) \
    $(wildcard include/config/mp/mstar/str/base.h) \
    $(wildcard include/config/mp/mstar/str/base/debug.h) \
    $(wildcard include/config/mp/ca7/quad/core/patch.h) \
    $(wildcard include/config/mp/ca7/quad/core/patch/debug.h) \
    $(wildcard include/config/mp/security/dm/verity.h) \
    $(wildcard include/config/mp/security/dm/verity/debug.h) \
    $(wildcard include/config/mp/sync/3/1/10/setting/vmalloc/setting.h) \
    $(wildcard include/config/mp/sync/3/1/10/setting/vmalloc/setting/debug.h) \
    $(wildcard include/config/mp/cma/patch/lx/memory/align/to/8k/check.h) \
    $(wildcard include/config/mp/cma/patch/lx/memory/align/to/8k/check/debug.h) \
    $(wildcard include/config/mp/cma/patch/cma/mstar/driver/buffer.h) \
    $(wildcard include/config/mp/cma/patch/cma/mstar/driver/buffer/debug.h) \
    $(wildcard include/config/mp/cma/patch/cma/aggressive/alloc.h) \
    $(wildcard include/config/mp/cma/patch/cma/aggressive/alloc/debug.h) \
    $(wildcard include/config/mp/cma/patch/cma/more/aggressive/alloc.h) \
    $(wildcard include/config/mp/cma/patch/cma/more/aggressive/alloc/debug.h) \
    $(wildcard include/config/mp/cma/patch/cma/dynamic/strategy.h) \
    $(wildcard include/config/mp/cma/patch/cma/dynamic/strategy/debug.h) \
    $(wildcard include/config/mp/cma/patch/cma/64/bit/temp/modification.h) \
    $(wildcard include/config/mp/cma/patch/cma/64/bit/temp/modification/debug.h) \
    $(wildcard include/config/mp/cma/patch/cma/default/buffer/limitted/to/lx0.h) \
    $(wildcard include/config/mp/cma/patch/cma/default/buffer/limitted/to/lx0/debug.h) \
    $(wildcard include/config/mp/cma/patch/force/migration.h) \
    $(wildcard include/config/mp/cma/patch/force/migration/debug.h) \
    $(wildcard include/config/mp/cma/patch/force/alloc/start/addr.h) \
    $(wildcard include/config/mp/cma/patch/force/alloc/start/addr/debug.h) \
    $(wildcard include/config/mp/cma/patch/compaction/from/noncma/to/cma.h) \
    $(wildcard include/config/mp/cma/patch/compaction/from/noncma/to/cma/debug.h) \
    $(wildcard include/config/mp/cma/patch/delay/free.h) \
    $(wildcard include/config/mp/cma/patch/delay/free/debug.h) \
    $(wildcard include/config/mp/cma/patch/migration/filter.h) \
    $(wildcard include/config/mp/cma/patch/migration/filter/debug.h) \
    $(wildcard include/config/mp/cma/patch/forbidden/movable/fallback/to/cma.h) \
    $(wildcard include/config/mp/cma/patch/forbidden/movable/fallback/to/cma/debug.h) \
    $(wildcard include/config/mp/cma/patch/debug/static/miu/protect.h) \
    $(wildcard include/config/mp/cma/patch/debug/static/miu/protect/debug.h) \
    $(wildcard include/config/mp/cma/patch/oom/killer.h) \
    $(wildcard include/config/mp/cma/patch/oom/killer/debug.h) \
    $(wildcard include/config/mp/cma/patch/smaller/socket/buffer.h) \
    $(wildcard include/config/mp/cma/patch/smaller/socket/buffer/debug.h) \
    $(wildcard include/config/mp/cma/patch/do/fork/page/pool.h) \
    $(wildcard include/config/mp/cma/patch/do/fork/page/pool/debug.h) \
    $(wildcard include/config/mp/cma/patch/smaller/slab/page.h) \
    $(wildcard include/config/mp/cma/patch/smaller/slab/page/debug.h) \
    $(wildcard include/config/mp/cma/patch/ion/low/order/alloc.h) \
    $(wildcard include/config/mp/cma/patch/ion/low/order/alloc/debug.h) \
    $(wildcard include/config/mp/cma/patch/disable/hiorder.h) \
    $(wildcard include/config/mp/cma/patch/disable/hiorder/debug.h) \
    $(wildcard include/config/mp/cma/patch/mstar/kmalloc.h) \
    $(wildcard include/config/mp/cma/patch/mstar/kmalloc/debug.h) \
    $(wildcard include/config/mp/cma/patch/agressive/kill/process/to/free/cma/page.h) \
    $(wildcard include/config/mp/cma/patch/agressive/kill/process/to/free/cma/page/debug.h) \
    $(wildcard include/config/mp/cma/patch/ksm/migration/failure.h) \
    $(wildcard include/config/mp/cma/patch/ksm/migration/failure/debug.h) \
    $(wildcard include/config/mp/cma/patch/vm/unmap.h) \
    $(wildcard include/config/mp/cma/patch/vm/unmap/debug.h) \
    $(wildcard include/config/mp/cma/patch/evdev/using/cma/buffer.h) \
    $(wildcard include/config/mp/cma/patch/evdev/using/cma/buffer/debug.h) \
    $(wildcard include/config/mp/cma/patch/sparsemem/mapping/256mb.h) \
    $(wildcard include/config/mp/cma/patch/sparsemem/mapping/256mb/debug.h) \
    $(wildcard include/config/mp/cma/patch/mboot/str/use/cma.h) \
    $(wildcard include/config/mp/cma/patch/mboot/str/use/cma/debug.h) \
    $(wildcard include/config/mp/cma/patch/low/memeroy/solution.h) \
    $(wildcard include/config/mp/cma/patch/low/memeroy/solution/debug.h) \
    $(wildcard include/config/mp/cma/patch/count/timecost.h) \
    $(wildcard include/config/mp/cma/patch/count/timecost/debug.h) \
    $(wildcard include/config/mp/cma/patch/pool/utopia/to/kernel.h) \
    $(wildcard include/config/mp/cma/patch/pool/utopia/to/kernel/debug.h) \
    $(wildcard include/config/mp/cma/patch/pci/alloc/free/with/cma.h) \
    $(wildcard include/config/mp/cma/patch/pci/alloc/free/with/cma/debug.h) \
    $(wildcard include/config/mp/global/timer/12mhz/patch.h) \
    $(wildcard include/config/mp/global/timer/12mhz/patch/debug.h) \
    $(wildcard include/config/mp/ion/patch/mstar.h) \
    $(wildcard include/config/mp/ion/patch/mstar/debug.h) \
    $(wildcard include/config/mp/ion/patch/cache/flush/mod.h) \
    $(wildcard include/config/mp/ion/patch/cache/flush/mod/debug.h) \
    $(wildcard include/config/mp/static/timer/clock/source.h) \
    $(wildcard include/config/mp/static/timer/clock/source/debug.h) \
    $(wildcard include/config/mp/sparse/mem/enable/holes/in/zone/check.h) \
    $(wildcard include/config/mp/sparse/mem/enable/holes/in/zone/check/debug.h) \
    $(wildcard include/config/mp/fat/volume/label.h) \
    $(wildcard include/config/mp/fat/volume/label/debug.h) \
    $(wildcard include/config/mp/fat/debug/message/control.h) \
    $(wildcard include/config/mp/fat/debug/message/control/debug.h) \
    $(wildcard include/config/mp/ntfs/ioctl.h) \
    $(wildcard include/config/mp/ntfs/ioctl/debug.h) \
    $(wildcard include/config/mp/ntfs/volume/label.h) \
    $(wildcard include/config/mp/ntfs/volume/label/debug.h) \
    $(wildcard include/config/mp/ntfs/volume/id.h) \
    $(wildcard include/config/mp/ntfs/volume/id/debug.h) \
    $(wildcard include/config/mp/ntfs/read/pages.h) \
    $(wildcard include/config/mp/ntfs/read/pages/debug.h) \
    $(wildcard include/config/mp/ntfs/2tb/plus/partition/support.h) \
    $(wildcard include/config/mp/ntfs/2tb/plus/partition/support/debug.h) \
    $(wildcard include/config/mp/ntfs/page/cache/readahead.h) \
    $(wildcard include/config/mp/ntfs/page/cache/readahead/debug.h) \
    $(wildcard include/config/mp/scsi/mstar/sd/card/hotplug.h) \
    $(wildcard include/config/mp/scsi/mstar/sd/card/hotplug/debug.h) \
    $(wildcard include/config/mp/scsi/mstar/sd/card/immediately/unplug.h) \
    $(wildcard include/config/mp/scsi/mstar/sd/card/immediately/unplug/debug.h) \
    $(wildcard include/config/mp/scsi/hd/suspend.h) \
    $(wildcard include/config/mp/scsi/hd/suspend/debug.h) \
    $(wildcard include/config/mp/scsi/multi/lun.h) \
    $(wildcard include/config/mp/scsi/multi/lun/debug.h) \
    $(wildcard include/config/mp/ethernet/mstar/icmp/enhance.h) \
    $(wildcard include/config/mp/ethernet/mstar/icmp/enhance/debug.h) \
    $(wildcard include/config/mp/wifi/inc/default/dma/coherent/pool/size.h) \
    $(wildcard include/config/mp/wifi/inc/default/dma/coherent/pool/size/debug.h) \
    $(wildcard include/config/mp/cifs/unuse/ntlmssp/default.h) \
    $(wildcard include/config/mp/cifs/unuse/ntlmssp/default/debug.h) \
    $(wildcard include/config/mp/sched/policy/patch.h) \
    $(wildcard include/config/mp/sched/policy/patch/debug.h) \
    $(wildcard include/config/mp/sched/policy/changed/avoided.h) \
    $(wildcard include/config/mp/sched/policy/changed/avoided/debug.h) \
    $(wildcard include/config/mp/pure/sn/32bit.h) \
    $(wildcard include/config/mp/pure/sn/32bit/debug.h) \
    $(wildcard include/config/mp/new/utopia/32bit.h) \
    $(wildcard include/config/mp/new/utopia/32bit/debug.h) \
    $(wildcard include/config/mp/dvfs/force/set/target/freq.h) \
    $(wildcard include/config/mp/dvfs/force/set/target/freq/debug.h) \
    $(wildcard include/config/mp/dvfs/force/use/one/freq.h) \
    $(wildcard include/config/mp/dvfs/force/use/one/freq/debug.h) \
    $(wildcard include/config/mp/dvfs/force/pingo/test.h) \
    $(wildcard include/config/mp/dvfs/force/pingo/test/debug.h) \
    $(wildcard include/config/mp/dvfs/flow/debug/message.h) \
    $(wildcard include/config/mp/dvfs/flow/debug/message/debug.h) \
    $(wildcard include/config/mp/dvfs/vid/only.h) \
    $(wildcard include/config/mp/dvfs/vid/only/debug.h) \
    $(wildcard include/config/mp/dvfs/cpuhotplug.h) \
    $(wildcard include/config/mp/dvfs/cpuhotplug/debug.h) \
    $(wildcard include/config/mp/dvfs/cpuhotplug/use/online/cpu/max/load.h) \
    $(wildcard include/config/mp/dvfs/cpuhotplug/use/online/cpu/max/load/debug.h) \
    $(wildcard include/config/mp/dvfs/cpuhotplug/use/online/cpu/average/load.h) \
    $(wildcard include/config/mp/dvfs/cpuhotplug/use/online/cpu/average/load/debug.h) \
    $(wildcard include/config/mp/dvfs/ondemand/high/performance.h) \
    $(wildcard include/config/mp/dvfs/ondemand/high/performance/debug.h) \
    $(wildcard include/config/mp/dvfs/antutu/hotplug.h) \
    $(wildcard include/config/mp/dvfs/antutu/hotplug/debug.h) \
    $(wildcard include/config/mp/dvfs/interactive/patch.h) \
    $(wildcard include/config/mp/dvfs/interactive/patch/debug.h) \
    $(wildcard include/config/mp/dvfs/max/cpu/freq/override.h) \
    $(wildcard include/config/mp/dvfs/max/cpu/freq/override/debug.h) \
    $(wildcard include/config/mp/cache/drop.h) \
    $(wildcard include/config/mp/cache/drop/debug.h) \
    $(wildcard include/config/mp/antutu/mstar/hide/sched/policy.h) \
    $(wildcard include/config/mp/antutu/mstar/hide/sched/policy/debug.h) \
    $(wildcard include/config/mp/zram/zram/using/cma/pages.h) \
    $(wildcard include/config/mp/zram/zram/using/cma/pages/debug.h) \
    $(wildcard include/config/mp/kernel/compat/patch/fix/inode/cluster/list.h) \
    $(wildcard include/config/mp/kernel/compat/patch/fix/inode/cluster/list/debug.h) \
    $(wildcard include/config/mp/page/global/directory/speedup.h) \
    $(wildcard include/config/mp/page/global/directory/speedup/debug.h) \
    $(wildcard include/config/mp/pci/patch/addr/translate.h) \
    $(wildcard include/config/mp/pci/patch/addr/translate/debug.h) \
    $(wildcard include/config/mp/pci/mstar.h) \
    $(wildcard include/config/mp/pci/mstar/debug.h) \
    $(wildcard include/config/mp/ddi/64bit/kernel/apply/32bit/binder/api.h) \
    $(wildcard include/config/mp/ddi/64bit/kernel/apply/32bit/binder/api/debug.h) \
    $(wildcard include/config/mp/xc/hdr/1/4/0.h) \
    $(wildcard include/config/mp/xc/hdr/1/4/0/debug.h) \
    $(wildcard include/config/mp/xc/hdr/1/4/2.h) \
    $(wildcard include/config/mp/xc/hdr/1/4/2/debug.h) \
    $(wildcard include/config/mp/printk/no/console/verbose.h) \
    $(wildcard include/config/mp/printk/no/console/verbose/debug.h) \
    $(wildcard include/config/mp/seqlock/rcu/stall.h) \
    $(wildcard include/config/mp/seqlock/rcu/stall/debug.h) \
    $(wildcard include/config/mp/r2/str/enable.h) \
    $(wildcard include/config/mp/r2/str/enable/debug.h) \
    $(wildcard include/config/mp/hrt/timer/enable.h) \
    $(wildcard include/config/mp/hrt/timer/enable/debug.h) \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/arch_hweight.h \
  include/asm-generic/bitops/const_hweight.h \
  include/asm-generic/bitops/lock.h \
  include/asm-generic/bitops/le.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/uapi/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/uapi/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  include/uapi/linux/swab.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/swab.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/uapi/asm/swab.h \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/ext2-atomic-setbit.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/printk.h \
    $(wildcard include/config/early/printk.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
  include/linux/init.h \
    $(wildcard include/config/broken/rodata.h) \
  include/linux/kern_levels.h \
  include/linux/dynamic_debug.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  include/uapi/linux/string.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/string.h \
  include/linux/errno.h \
  include/uapi/linux/errno.h \
  arch/arm/include/generated/asm/errno.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/uapi/asm-generic/errno.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/uapi/asm-generic/errno-base.h \
  include/uapi/linux/kernel.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/uapi/linux/sysinfo.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/div64.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/compiler.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/thumb2/kernel.h) \
    $(wildcard include/config/debug/bugverbose.h) \
    $(wildcard include/config/arm/lpae.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/cache.h \
    $(wildcard include/config/arm/l1/cache/shift.h) \
    $(wildcard include/config/aeabi.h) \
  include/linux/seqlock.h \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/context/tracking.h) \
    $(wildcard include/config/preempt/count.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  arch/arm/include/generated/asm/preempt.h \
  include/asm-generic/preempt.h \
  include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  include/linux/bug.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/relaxed.h \
  include/asm-generic/relaxed.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/thread_info.h \
    $(wildcard include/config/crunch.h) \
    $(wildcard include/config/arm/thumbee.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/fpstate.h \
    $(wildcard include/config/vfpv3.h) \
    $(wildcard include/config/iwmmxt.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
    $(wildcard include/config/cpu/use/domains.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/barrier.h \
    $(wildcard include/config/cpu/32v6k.h) \
    $(wildcard include/config/cpu/xsc3.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/arch/has/barriers.h) \
    $(wildcard include/config/arm/dma/mem/bufferable.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/outercache.h \
    $(wildcard include/config/outer/cache/sync.h) \
    $(wildcard include/config/outer/cache.h) \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/spinlock_types.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/prove/rcu.h) \
  include/linux/rwlock_types.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/spinlock.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/processor.h \
    $(wildcard include/config/have/hw/breakpoint.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/arm/errata/754327.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/hw_breakpoint.h \
  include/asm-generic/processor.h \
  include/linux/rwlock.h \
  include/linux/spinlock_api_smp.h \
    $(wildcard include/config/inline/spin/lock.h) \
    $(wildcard include/config/inline/spin/lock/bh.h) \
    $(wildcard include/config/inline/spin/lock/irq.h) \
    $(wildcard include/config/inline/spin/lock/irqsave.h) \
    $(wildcard include/config/inline/spin/trylock.h) \
    $(wildcard include/config/inline/spin/trylock/bh.h) \
    $(wildcard include/config/uninline/spin/unlock.h) \
    $(wildcard include/config/inline/spin/unlock/bh.h) \
    $(wildcard include/config/inline/spin/unlock/irq.h) \
    $(wildcard include/config/inline/spin/unlock/irqrestore.h) \
  include/linux/rwlock_api_smp.h \
    $(wildcard include/config/inline/read/lock.h) \
    $(wildcard include/config/inline/write/lock.h) \
    $(wildcard include/config/inline/read/lock/bh.h) \
    $(wildcard include/config/inline/write/lock/bh.h) \
    $(wildcard include/config/inline/read/lock/irq.h) \
    $(wildcard include/config/inline/write/lock/irq.h) \
    $(wildcard include/config/inline/read/lock/irqsave.h) \
    $(wildcard include/config/inline/write/lock/irqsave.h) \
    $(wildcard include/config/inline/read/trylock.h) \
    $(wildcard include/config/inline/write/trylock.h) \
    $(wildcard include/config/inline/read/unlock.h) \
    $(wildcard include/config/inline/write/unlock.h) \
    $(wildcard include/config/inline/read/unlock/bh.h) \
    $(wildcard include/config/inline/write/unlock/bh.h) \
    $(wildcard include/config/inline/read/unlock/irq.h) \
    $(wildcard include/config/inline/write/unlock/irq.h) \
    $(wildcard include/config/inline/read/unlock/irqrestore.h) \
    $(wildcard include/config/inline/write/unlock/irqrestore.h) \
  include/linux/atomic.h \
    $(wildcard include/config/arch/has/atomic/or.h) \
    $(wildcard include/config/generic/atomic64.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/atomic.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/cmpxchg.h \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
    $(wildcard include/config/cpu/v6.h) \
  include/asm-generic/cmpxchg-local.h \
  include/asm-generic/atomic-long.h \
  include/linux/math64.h \
  include/uapi/linux/time.h \
  include/linux/uidgid.h \
    $(wildcard include/config/uidgid/strict/type/checks.h) \
    $(wildcard include/config/user/ns.h) \
  include/linux/highuid.h \
  include/linux/kmod.h \
  include/linux/gfp.h \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
    $(wildcard include/config/pm/sleep.h) \
    $(wildcard include/config/cma.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/memory/isolation.h) \
    $(wildcard include/config/memcg.h) \
    $(wildcard include/config/compaction.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/have/memblock/node/map.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/no/bootmem.h) \
    $(wildcard include/config/numa/balancing.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/have/memoryless/nodes.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/have/arch/pfn/valid.h) \
    $(wildcard include/config/nodes/span/other/nodes.h) \
    $(wildcard include/config/holes/in/zone.h) \
    $(wildcard include/config/arch/has/holes/memorymodel.h) \
  include/linux/wait.h \
  arch/arm/include/generated/asm/current.h \
  include/asm-generic/current.h \
  include/uapi/linux/wait.h \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/linux/nodemask.h \
    $(wildcard include/config/movable/node.h) \
  include/linux/bitmap.h \
  include/linux/pageblock-flags.h \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/hugetlb/page/size/variable.h) \
  include/linux/page-flags-layout.h \
    $(wildcard include/config/sparsemem/vmemmap.h) \
  include/generated/bounds.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/sparsemem.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/memory.h \
    $(wildcard include/config/need/mach/memory/h.h) \
    $(wildcard include/config/page/offset.h) \
    $(wildcard include/config/task/size.h) \
    $(wildcard include/config/dram/size.h) \
    $(wildcard include/config/dram/base.h) \
    $(wildcard include/config/have/tcm.h) \
    $(wildcard include/config/arm/patch/phys/virt.h) \
    $(wildcard include/config/phys/offset.h) \
    $(wildcard include/config/virt/to/bus.h) \
  include/linux/sizes.h \
  arch/arm/arm-boards/../../../../mstar2/hal/k6lite/cpu//include/mach/memory.h \
    $(wildcard include/config/memory/start/address.h) \
  include/asm-generic/memory_model.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/page.h \
    $(wildcard include/config/cpu/copy/v4wt.h) \
    $(wildcard include/config/cpu/copy/v4wb.h) \
    $(wildcard include/config/cpu/copy/feroceon.h) \
    $(wildcard include/config/cpu/copy/fa.h) \
    $(wildcard include/config/cpu/xscale.h) \
    $(wildcard include/config/cpu/copy/v6.h) \
    $(wildcard include/config/kuser/helpers.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/glue.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/pgtable-2level-types.h \
  include/asm-generic/getorder.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/memory/hotremove.h) \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
    $(wildcard include/config/have/bootmem/info/node.h) \
  include/linux/notifier.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
    $(wildcard include/config/mutex/spin/on/owner.h) \
    $(wildcard include/config/have/arch/mutex/cpu/relax.h) \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/linux/srcu.h \
  include/linux/rcupdate.h \
    $(wildcard include/config/rcu/torture/test.h) \
    $(wildcard include/config/tree/rcu.h) \
    $(wildcard include/config/tree/preempt/rcu.h) \
    $(wildcard include/config/rcu/trace.h) \
    $(wildcard include/config/preempt/rcu.h) \
    $(wildcard include/config/rcu/user/qs.h) \
    $(wildcard include/config/tiny/rcu.h) \
    $(wildcard include/config/tiny/preempt/rcu.h) \
    $(wildcard include/config/debug/objects/rcu/head.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/rcu/nocb/cpu.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
  include/linux/completion.h \
  include/linux/debugobjects.h \
    $(wildcard include/config/debug/objects.h) \
    $(wildcard include/config/debug/objects/free.h) \
  include/linux/rcutree.h \
  include/linux/workqueue.h \
    $(wildcard include/config/debug/objects/work.h) \
    $(wildcard include/config/freezer.h) \
  include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
    $(wildcard include/config/debug/objects/timers.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/jiffies.h \
  include/linux/timex.h \
  include/uapi/linux/timex.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/uapi/linux/param.h \
  arch/arm/include/generated/asm/param.h \
  include/asm-generic/param.h \
    $(wildcard include/config/hz.h) \
  include/uapi/asm-generic/param.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/timex.h \
    $(wildcard include/config/arch/multiplatform.h) \
  arch/arm/arm-boards/../../../../mstar2/hal/k6lite/cpu//include/mach/timex.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
    $(wildcard include/config/sched/book.h) \
    $(wildcard include/config/use/percpu/numa/node/id.h) \
  include/linux/smp.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/smp.h \
  include/linux/percpu.h \
    $(wildcard include/config/need/per/cpu/embed/first/chunk.h) \
    $(wildcard include/config/need/per/cpu/page/first/chunk.h) \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  include/linux/pfn.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/linux/percpu-defs.h \
    $(wildcard include/config/debug/force/weak/per/cpu.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/topology.h \
    $(wildcard include/config/arm/cpu/topology.h) \
  include/asm-generic/topology.h \
  include/linux/mmdebug.h \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/debug/virtual.h) \
  include/linux/sysctl.h \
    $(wildcard include/config/sysctl.h) \
  include/linux/rbtree.h \
  include/uapi/linux/sysctl.h \
  include/linux/elf.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/elf.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/user.h \
  include/uapi/linux/elf.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/include/uapi/linux/elf-em.h \
  include/linux/kobject.h \
  include/linux/sysfs.h \
  include/linux/kobject_ns.h \
  include/linux/kref.h \
  include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/ppc64.h) \
  include/linux/tracepoint.h \
  include/linux/static_key.h \
  include/linux/jump_label.h \
    $(wildcard include/config/jump/label.h) \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/jump_label.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/system.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/switch_to.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/system_info.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/system_misc.h \
  /home/wenshuai.xi/SI/alkaid/project/kbuild/3.10.40/arch/arm/include/asm/module.h \
    $(wildcard include/config/arm/unwind.h) \
  include/asm-generic/module.h \
    $(wildcard include/config/have/mod/arch/specific.h) \
    $(wildcard include/config/modules/use/elf/rel.h) \
    $(wildcard include/config/modules/use/elf/rela.h) \
  include/linux/vermagic.h \
  include/generated/utsrelease.h \

/home/wenshuai.xi/SI/alkaid/sdk/mhal/k6l/mhal.mod.o: $(deps_/home/wenshuai.xi/SI/alkaid/sdk/mhal/k6l/mhal.mod.o)

$(deps_/home/wenshuai.xi/SI/alkaid/sdk/mhal/k6l/mhal.mod.o):
