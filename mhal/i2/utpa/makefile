BUILD_DIR:=build

all_release: build/utpa2k.ko
	ln -sf $(CURDIR)/mxlib/include ../../include/utopia
	cp -f build/utpa2k.ko $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)
	ln -sf build/Module.symvers ./
	cd $(BUILD_DIR); $(MAKE) -f $(CURDIR)/fetch_macros.mk setupmacro

clean:
	rm -f $(CURDIR)/mxlib/include/utopia_macros.h
	rm -f ../../include/utopia
	cd $(BUILD_DIR); $(MAKE) clean

build/utpa2k.ko:
	./genlink.sh
	cd $(BUILD_DIR); cp i2/.config_i2_linux_arm_hardfloat_dynamic_general_32bit_ko .config
	cd $(BUILD_DIR); $(MAKE) defconfig
	cd $(BUILD_DIR); $(MAKE) kdrv KERNELDIR=$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)
