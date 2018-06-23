include ../../project/configs/current.configs

include $(PROJ_ROOT)/release/customer_tailor/$(CUSTOMER_TAILOR)
fetch-value=$(foreach n,$1,$($(n)) $($(n)enable))

INTERFACE_MODULES:=
MHAL_MODULES:=

include src/*/config.mk $(foreach n,$(MHAL),../mhal/$(n)/*/config.mk)

INTERFACE_ENABLED:=$(patsubst %_enable_,%,$(patsubst %__,%,$(filter-out %_disable_, $(foreach n,$(INTERFACE_MODULES),$(n)_$(interface_$(n))_))))
INTERFACE_DISABLED:=$(filter-out $(INTERFACE_ENABLED),$(INTERFACE_MODULES))
MHAL_ENABLED:=$(patsubst %_enable_,%,$(patsubst %__,%,$(filter-out %_disable_, $(foreach n,$(MHAL_MODULES),$(n)_$(mhal_$(n))_))))
MHAL_DISABLED:=$(filter-out $(MHAL_ENABLED),$(MHAL_MODULES))
CONFIG_LIST:=$(INTERFACE_ENABLED)

TARGET_MODULES := $(patsubst %, %_module, $(CONFIG_LIST))
TARGET_MODULES_INSTALL := $(patsubst %, %_module_install, $(CONFIG_LIST))
TARGET_LIBS:= $(patsubst %, %_lib, $(CONFIG_LIST))
TARGET_LIBS_INSTALL := $(patsubst %, %_lib_install, $(CONFIG_LIST))
TARGET_CLEAN := $(patsubst %, %_clean, $(CONFIG_LIST))
TARGET_DISTCLEAN := $(patsubst %, %_distclean, $(CONFIG_LIST))
TARGET_MAKEFILE := $(patsubst %, %_makefile, $(CONFIG_LIST))
TARGET_MODULES_DEPEND := $(patsubst %, %_depend, $(CONFIG_LIST))
TARGET_MODULES_DOT_DEPEND := $(patsubst %, src/%/.mod_depend, $(CONFIG_LIST))

.PHONY: $(CONFIG_LIST)

MOD_PREFIX:=mi_

include $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk

export PROJ_ROOT CHIP PRODUCT BOARD TOOLCHAIN TOOLCHAIN_VERSION KERNEL_VERSION CUSTOMER_OPTIONS MOD_PREFIX INTERFACE_ENABLED INTERFACE_DISABLED MHAL_ENABLED MHAL_DISABLED

all: $(CONFIG_LIST)
	@echo all done!

clean: $(TARGET_CLEAN)
	@echo all clean!

distclean: $(TARGET_DISTCLEAN)
	@rm -f .mods_depend
	@echo all clean!

install: $(TARGET_MODULES_INSTALL) $(TARGET_LIBS_INSTALL)
	@$(MAKE) gen_depend
	@cp .mods_depend $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/

install_with_depend: $(TARGET_MODULES_INSTALL) $(TARGET_LIBS_INSTALL)
	@cp .mods_depend $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/

modules_install: $(TARGET_MODULES_INSTALL)

libs_install: $(TARGET_LIBS_INSTALL)

libs: $(TARGET_LIBS)
	@echo wrapper done!

all_with_depend:
	@$(MAKE) $(TARGET_MODULES_DEPEND)

do_nothing:
	@#do nothing

$(TARGET_MODULES_DEPEND):%_depend:
	@$(MAKE) $(foreach n,$($(patsubst %_depend,%,$@)_DEP_MODULE),$(n)_depend) do_nothing
	@$(MAKE) $(patsubst %_depend,%,$@)
	@$(MAKE) src/$(patsubst %_depend,%,$@)/.mod_depend

gen_depend:
	@$(foreach n,$(TARGET_MODULES_DOT_DEPEND), $(MAKE) $(n);)

$(TARGET_MODULES_DOT_DEPEND):src/%/.mod_depend:
	@$(foreach n, $($(patsubst src/%/.mod_depend,%,$@)_DEP_MODULE), $(MAKE) src/$(n)/.mod_depend ;)
	@touch $@
	@echo $(MOD_PREFIX)$(patsubst src/%/.mod_depend,%,$@) >> .mods_depend

$(CONFIG_LIST):%:
	@$(MAKE) $@_module WRAPPER_FILE=$($@_WRAPPER_FILE)
	@$(MAKE) $@_lib

$(TARGET_MODULES): %_module: %_makefile
ifneq ($(WRAPPER_FILE),)
	$(MAKE) -C src/$(patsubst %_module,%,$@) module MODULE_DIR=$(patsubst %_module,%,$@) DEP_MODULE="$($(patsubst %_module,%,$@)_DEP_MODULE)" DEP_HAL="$($(patsubst %_module,%,$@)_DEP_HAL)" API_FILE=$($(patsubst %_module,%,$@)_API_FILE) WRAPPER_FILE=$($(patsubst %_module,%,$@)_WRAPPER_FILE) KINTERNAL_FILES="$($(patsubst %_module,%,$@)_KINTERNAL_FILES)" IMPL_FILE="$($(patsubst %_module,%,$@)_IMPL_FILE)" KAPI_DISABLE=$($(patsubst %_module,%,$@)_KAPI_DISABLE) DEP_EXT_INC="$($(patsubst %_module,%,$@)_DEP_EXT_INC)" DEP_HAL_INC="$($(patsubst %_module,%,$@)_DEP_HAL_INC)"
endif

$(TARGET_MODULES_INSTALL): %_module_install: %_makefile
	$(MAKE) -C src/$(patsubst %_module_install,%,$@) module_install MODULE_DIR=$(patsubst %_module_install,%,$@) DEP_MODULE="$($(patsubst %_module_install,%,$@)_DEP_MODULE)" API_FILE=$($(patsubst %_module_install,%,$@)_API_FILE) WRAPPER_FILE=$($(patsubst %_module_install,%,$@)_WRAPPER_FILE) IMPL_FILE="$($(patsubst %_module_install,%,$@)_IMPL_FILE)"

$(TARGET_LIBS): %_lib: %_makefile
	$(MAKE) -C src/$(patsubst %_lib,%,$@) lib MODULE_DIR=$(patsubst %_lib,%,$@) DEP_MODULE="$($(patsubst %_lib,%,$@)_DEP_MODULE)" API_FILE=$($(patsubst %_lib,%,$@)_API_FILE) CC=$(PREFIX)gcc UAPI_DISABLE=$($(patsubst %_lib,%,$@)_UAPI_DISABLE) USER_INC=$($(patsubst %_lib,%,$@)_USER_INC) UINTERNAL_FILES="$($(patsubst %_lib,%,$@)_UINTERNAL_FILES)" DEP_EXT_INC="$($(patsubst %_lib,%,$@)_DEP_EXT_INC)"

$(TARGET_LIBS_INSTALL): %_lib_install: %_makefile
	$(MAKE) -C src/$(patsubst %_lib_install,%,$@) lib_install MODULE_DIR=$(patsubst %_lib_install,%,$@) API_FILE=$($(patsubst %_lib_install,%,$@)_API_FILE) TARGET_INCLUDEDIR=$(PROJ_ROOT)/release/$(PRODUCT)/include TARGET_LIBDIR=$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib UAPI_DISABLE=$($(patsubst %_lib_install,%,$@)_UAPI_DISABLE) DEP_EXT_INC="$($(patsubst %_lib_install,%,$@)_DEP_EXT_INC)"

$(TARGET_CLEAN): %_clean: %_makefile
	$(MAKE) -C src/$(patsubst %_clean,%,$@) module_clean lib_clean MODULE_DIR=$(patsubst %_clean,%,$@) IMPL_FILE="$($(patsubst %_clean,%,$@)_IMPL_FILE)"

$(TARGET_DISTCLEAN): %_distclean: %_makefile
	$(MAKE) -C src/$(patsubst %_distclean,%,$@) module_clean lib_clean MODULE_DIR=$(patsubst %_distclean,%,$@) IMPL_FILE="$($(patsubst %_distclean,%,$@)_IMPL_FILE)"
	rm -f src/$(patsubst %_distclean,%,$@)/Makefile src/$(patsubst %_distclean,%,$@)/.mod_depend

$(TARGET_MAKEFILE): %_makefile:
	@ln -sf $(CURDIR)/modules.mk src/$(patsubst %_makefile,%,$@)/Makefile
