.PHONY: verify interface mhal misc install

SOURCE_RELEASE_DEPEND:=install_with_depend misc_install
SOURCE_RELEASE:=install misc_install
SOURCE_CLEAN:=clean_all

install: all_build
	$(MAKE) -C $(PROJ_ROOT)/../sdk/mhal install
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify install

install_with_depend: all_build_with_depend
	$(MAKE) -C $(PROJ_ROOT)/../sdk/mhal install
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify install

all_build: verify interface mhal
	$(MAKE) misc

all_build_with_depend: verify_with_depend interface_with_depend mhal
	$(MAKE) misc

verify: interface_install
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify all

verify_with_depend: interface_install_with_depend
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify all

interface_install: interface
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface install

interface_install_with_depend: interface_with_depend
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface install_with_depend

interface_with_depend: mhal
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface all_with_depend

interface: mhal
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface all

mhal: $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config
	$(MAKE) -C $(PROJ_ROOT)/../sdk/mhal all

misc: $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config
	$(MAKE) -C $(PROJ_ROOT)/../sdk/misc all

misc_install: misc
	$(MAKE) -C $(PROJ_ROOT)/../sdk/misc install

clean_all: clean_interface clean_mhal
	$(MAKE) -C $(PROJ_ROOT)/../sdk/misc clean
	rm -f $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config
	rm -f $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/Module.symvers
	rm -f $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/config
	rm -f $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/generated
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify clean

clean_interface:$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface distclean

clean_mhal:$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config
	$(MAKE) -C $(PROJ_ROOT)/../sdk/mhal clean

$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config:
	$(MAKE) -C $(PROJ_ROOT) kbuild/$(KERNEL_VERSION)/.config
