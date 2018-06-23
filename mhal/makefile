include $(CURDIR)/../../project/configs/current.configs

include $(PROJ_ROOT)/release/customer_tailor/$(CUSTOMER_TAILOR)
fetch-value=$(foreach n,$1,$($(n)) $($(n)enable))

MHAL_MODULES:=
HM_LIST:=

include $(foreach n,$(MHAL),$(n)/*/config.mk)

HM_LIST:=$(patsubst %_enable_,%,$(patsubst %__,%,$(filter-out %_disable_, $(foreach n,$(HM_LIST),$(n)_$($(n))_))))
MHAL_ENABLED:=$(patsubst %_enable_,%,$(patsubst %__,%,$(filter-out %_disable_, $(foreach n,$(MHAL_MODULES),$(n)_$(mhal_$(n))_))))
MHAL_DISABLED:=$(filter-out $(MHAL_ENABLED),$(MHAL_MODULES))
HM_LIST:=$(filter-out $(foreach n,$(MHAL_DISABLED),%/$(n)),$(HM_LIST))

TARGET_ALL := $(foreach n, $(HM_LIST), $(n)_all)
ALL_TARGET := $(foreach n,$(HM_LIST),$($(n)_STYLE))
TARGET_CLEAN := $(foreach n, $(HM_LIST), $(n)_clean)
CLEAN_TARGET := $(foreach n,$(ALL_TARGET),$(n)_clean)
TARGET_SIMPLE := $(filter %_simple, $(ALL_TARGET))
TARGET_SIMPLE_CLEAN := $(foreach n,$(TARGET_SIMPLE),$(n)_clean)
TARGET_SIMPLE_MAKEFILE := $(foreach n,$(TARGET_SIMPLE),$(n)_makefile)
TARGET_SIMPLE_HEADERS := $(foreach n,$(TARGET_SIMPLE),$(n)_headers)
TARGET_HAL := $(filter %_hal-style, $(ALL_TARGET))
TARGET_HAL_CLEAN := $(foreach n,$(TARGET_HAL),$(n)_clean)
TARGET_HAL_MAKEFILE := $(foreach n,$(TARGET_HAL),$(n)_makefile)
TARGET_HAL_HEADERS := $(foreach n,$(TARGET_HAL),$(n)_headers)
TARGET_RAW := $(filter %_raw-style, $(ALL_TARGET))
TARGET_RAW_CLEAN := $(foreach n,$(TARGET_RAW),$(n)_clean)
TARGET_RAW_MAKEFILE := $(foreach n,$(TARGET_RAW),$(n)_makefile)
TARGET_RAW_HEADERS := $(foreach n,$(TARGET_RAW),$(n)_headers)
ifeq ($(CHIP)/utpa, $(wildcard $(CHIP)/utpa))
TARGET_UTOPIA := include/utopia/utopia_macros.h
endif

export PROJ_ROOT CHIP PRODUCT BOARD KERNEL_VERSION UTOPIA TOOLCHAIN TOOLCHAIN_VERSION MHAL_ENABLED MHAL_DISABLED
include $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk

.PHONY: $(HM_LIST) $(TARGET_UTOPIA)

fetch-deps = $($(patsubst %_all,%,$(filter %$(patsubst %_ko,%,$1)_all, $(TARGET_ALL)))_DEPS)
conv-name-to-target = $(filter %$1_simple, $(TARGET_SIMPLE)) $(filter %$1_hal-style, $(TARGET_HAL)) $(filter %$1_raw-style, $(TARGET_RAW))
clean-ko = $(foreach n, $(foreach o,$1.ko $1.o $1.mod.o,$(o) .$(o).cmd) $1.mod.c modules.order Module.symvers .tmp_versions, $(CHIP)/$(n)) license.o .license.o.cmd

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    DEBUG=1
endif

help:
	@echo make all
	@echo make clean

$(foreach n,$(HM_LIST),$(notdir $(n))):
	$(MAKE) $(filter $(filter %$(@),$(HM_LIST))%, $(TARGET_ALL))
	$(MAKE) module

$(foreach n,$(HM_LIST),$(notdir $(n))_ko):
	@$(MAKE) $(foreach n,$(call fetch-deps,$@) $(patsubst %_ko,%,$@),$(call conv-name-to-target,$n))
	@ln -sf $(CURDIR)/kbuild.mk $(CHIP)/Makefile
	@$(MAKE) -C $(CHIP) module HAL_LIBS="$(patsubst %_simple,%/lib.a,$(filter $(foreach n,$(call fetch-deps,$@) $(patsubst %_ko,%,$@),%/$(n)_simple),$(TARGET_SIMPLE))) $(foreach n,$(patsubst %_raw-style,%,$(filter $(foreach n,$(call fetch-deps,$@) $(patsubst %_ko,%,$@),%/$(n)_raw-style),$(TARGET_RAW))) $(patsubst %_hal-style,%,$(filter $(foreach n,$(call fetch-deps,$@) $(patsubst %_ko,%,$@),%/$(n)_hal-style),$(TARGET_HAL))), $(n)/drv/lib.a $(n)/hal/lib.a)" MODULE_NAME=$(patsubst %_ko,%,$@)
	@rm -f $(CHIP)/Makefile

$(foreach n,$(HM_LIST),$(notdir $(n))_ko_clean):
	@rm -rf $(call clean-ko,$(patsubst %_ko_clean,%,$@))

$(foreach n,$(HM_LIST),$(notdir $(n))_all):
	$(MAKE) $(filter $(filter %$(patsubst %_all,%,$@),$(HM_LIST))%, $(TARGET_ALL))

$(foreach n,$(HM_LIST),$(notdir $(n))_clean):
	$(MAKE) $(filter $(filter %$(patsubst %_clean,%,$@),$(HM_LIST))%, $(TARGET_CLEAN))

$(TARGET_ALL): %_all:
	$(MAKE) $(filter $(patsubst %_all,%,$@)_%,$(ALL_TARGET))

$(TARGET_CLEAN): %_clean:
	$(MAKE) $(filter $(patsubst %_clean,%,$@)_%,$(CLEAN_TARGET))

$(patsubst %_raw-style,%/drv/Makefile,$(TARGET_RAW)) $(patsubst %_hal-style,%/drv/Makefile,$(TARGET_HAL)) $(patsubst %_raw-style,%/hal/Makefile,$(TARGET_RAW)) $(patsubst %_hal-style,%/hal/Makefile,$(TARGET_HAL)) $(patsubst %_simple,%/Makefile,$(TARGET_SIMPLE)):%/Makefile:
	@ln -sf $(CURDIR)/kbuild.mk $@

$(patsubst %_raw-style,%/drv/lib.a,$(TARGET_RAW)) $(patsubst %_hal-style,%/drv/lib.a,$(TARGET_HAL)):%/drv/lib.a:%/drv/Makefile
	$(MAKE) -C $(patsubst %/drv/lib.a,%,$@)/drv lib SRCS="$($(patsubst %/drv/lib.a,%,$@)_DRV_SRCS)" INCS="$($(patsubst %/drv/lib.a,%,$@)_DRV_INCS) $(foreach n,$($(patsubst %/drv/lib.a,%,$@)_DEPS),$(CURDIR)/include/$(n))" MACROS="$($(patsubst %/drv/lib.a,%,$@)_MACROS)"

$(patsubst %_raw-style,%/hal/lib.a,$(TARGET_RAW)) $(patsubst %_hal-style,%/hal/lib.a,$(TARGET_HAL)):%/hal/lib.a:%/hal/Makefile
	$(MAKE) -C $(patsubst %/hal/lib.a,%,$@)/hal lib SRCS="$($(patsubst %/hal/lib.a,%,$@)_HAL_SRCS)" INCS="$($(patsubst %/hal/lib.a,%,$@)_HAL_INCS) $(foreach n,$($(patsubst %/hal/lib.a,%,$@)_DEPS),$(CURDIR)/include/$(n))" MACROS="$($(patsubst %/hal/lib.a,%,$@)_MACROS)"

$(patsubst %_simple,%/lib.a,$(TARGET_SIMPLE)):%/lib.a:%/Makefile
	$(MAKE) -C $(patsubst %/lib.a,%,$@) lib SRCS="$($(patsubst %/lib.a,%,$@)_SRCS)" INCS="$(CURDIR)/include/common $($(patsubst %/lib.a,%,$@)_INCS) $(foreach n,$($(patsubst %/lib.a,%,$@)_DEPS),$(CURDIR)/include/$(n))" MACROS="$($(patsubst %/lib.a,%,$@)_MACROS)"

$(TARGET_SIMPLE): %_simple:
	$(MAKE) $(foreach n,$($(patsubst %_simple,%,$@)_DEPS),$(filter $(foreach n,$(filter %$(n),$(HM_LIST)),$(n)%), $(TARGET_SIMPLE_HEADERS) $(TARGET_HAL_HEADERS) $(TARGET_RAW_HEADERS))) $@_headers
	@rm -f $(patsubst %,$(patsubst %_simple,%,$@)/%,$($(patsubst %_simple,%,$@)_COMPILE_NAME))
	$(MAKE) $(patsubst %,$(patsubst %_simple,%,$@)/%,$($(patsubst %_simple,%,$@)_COMPILE_NAME)) nop

$(TARGET_SIMPLE_CLEAN): %_simple_clean:
	@rm -f $(foreach n,$(patsubst %.c, %.o, $($(patsubst %_simple_clean,%,$@)_SRCS)) built-in.o lib.a,$(patsubst %_simple_clean,%,$@)/$(n) $(patsubst %_simple_clean,%,$@)/$(dir $(n)).$(notdir $(n)).cmd)
	@rm -rf $(foreach n, Module.symvers modules.order .tmp_versions Makefile, $(patsubst %_simple_clean,%,$@)/$(n))
	@rm -rf include/$(notdir $(patsubst %_simple_clean,%,$@))

$(TARGET_HAL): %_hal-style:
	$(MAKE) $(foreach n,$($(patsubst %_hal-style,%,$@)_DEPS),$(filter $(foreach n,$(filter %$(n),$(HM_LIST)),$(n)%), $(TARGET_SIMPLE_HEADERS) $(TARGET_HAL_HEADERS) $(TARGET_RAW_HEADERS))) $@_headers
	@rm -f $(patsubst %_hal-style,%,$@)/hal/lib.a $(patsubst %_hal-style,%,$@)/drv/lib.a
	$(MAKE) $(patsubst %_hal-style,%,$@)/hal/lib.a $(patsubst %_hal-style,%,$@)/drv/lib.a

$(TARGET_HAL_CLEAN): %_hal-style_clean:
	@rm -f $(foreach n,$(patsubst %.c, %.o, $($(patsubst %_hal-style_clean,%,$@)_DRV_SRCS)) built-in.o lib.a,$(patsubst %_hal-style_clean,%,$@)/drv/$(n) $(patsubst %_hal-style_clean,%,$@)/drv/$(dir $(n)).$(notdir $(n)).cmd)
	@rm -rf $(foreach n, Module.symvers modules.order .tmp_versions Makefile, $(patsubst %_hal-style_clean,%,$@)/drv/$(n))
	@rm -f $(foreach n,$(patsubst %.c, %.o, $($(patsubst %_hal-style_clean,%,$@)_HAL_SRCS)) built-in.o lib.a,$(patsubst %_hal-style_clean,%,$@)/hal/$(n) $(patsubst %_hal-style_clean,%,$@)/hal/$(dir $(n)).$(notdir $(n)).cmd)
	@rm -rf $(foreach n, Module.symvers modules.order .tmp_versions Makefile, $(patsubst %_hal-style_clean,%,$@)/hal/$(n))
	@rm -rf include/$(notdir $(patsubst %_hal-style_clean,%,$@))

$(TARGET_RAW): %_raw-style:
	$(MAKE) $(foreach n,$($(patsubst %_raw-style,%,$@)_DEPS),$(filter $(foreach n,$(filter %$(n),$(HM_LIST)),$(n)%), $(TARGET_SIMPLE_HEADERS) $(TARGET_HAL_HEADERS) $(TARGET_RAW_HEADERS))) $@_headers
	@rm -f $(patsubst %_raw-style,%,$@)/hal/lib.a $(patsubst %_raw-style,%,$@)/drv/lib.a
	$(MAKE) $(patsubst %_raw-style,%,$@)/hal/lib.a $(patsubst %_raw-style,%,$@)/drv/lib.a

$(TARGET_RAW_CLEAN): %_raw-style_clean:
	@rm -f $(foreach n,$(patsubst %.c, %.o, $($(patsubst %_raw-style_clean,%,$@)_DRV_SRCS)) built-in.o lib.a,$(patsubst %_raw-style_clean,%,$@)/drv/$(n) $(patsubst %_raw-style_clean,%,$@)/drv/$(dir $(n)).$(notdir $(n)).cmd)
	@rm -rf $(foreach n, Module.symvers modules.order .tmp_versions Makefile, $(patsubst %_raw-style_clean,%,$@)/drv/$(n))
	@rm -f $(foreach n,$(patsubst %.c, %.o, $($(patsubst %_raw-style_clean,%,$@)_HAL_SRCS)) built-in.o lib.a,$(patsubst %_raw-style_clean,%,$@)/hal/$(n) $(patsubst %_raw-style_clean,%,$@)/hal/$(dir $(n)).$(notdir $(n)).cmd)
	@rm -rf $(foreach n, Module.symvers modules.order .tmp_versions Makefile, $(patsubst %_raw-style_clean,%,$@)/hal/$(n))
	@rm -rf include/$(notdir $(patsubst %_raw-style_clean,%,$@))

$(TARGET_SIMPLE_HEADERS): %_simple_headers:$(TARGET_UTOPIA)
	@rm -f include/$(notdir $(patsubst %_simple_headers,%,$@))
	@ln -sf $(CURDIR)/$(patsubst %_simple_headers,%,$@)/pub include/$(notdir $(patsubst %_simple_headers,%,$@))

$(TARGET_HAL_HEADERS): %_hal-style_headers:$(TARGET_UTOPIA)
	@mkdir -p include/$(notdir $(patsubst %_hal-style_headers,%,$@))
	@ln -sf $($(patsubst %_hal-style_headers,%,$@)_RELEASE_INC) include/$(notdir $(patsubst %_hal-style_headers,%,$@))

$(TARGET_RAW_HEADERS): %_raw-style_headers:$(TARGET_UTOPIA)
	@mkdir -p include/$(notdir $(patsubst %_raw-style_headers,%,$@))
	@ln -sf $($(patsubst %_raw-style_headers,%,$@)_RELEASE_INC) include/$(notdir $(patsubst %_raw-style_headers,%,$@))

$(TARGET_UTOPIA):
	@$(MAKE) -C $(CHIP)/utpa all_release

all: $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config
ifeq ($(CHIP)/utpa, $(wildcard $(CHIP)/utpa))
	$(MAKE) include/utopia/utopia_macros.h
endif
ifneq (include/common/mhal_common.h, $(wildcard include/common/mhal_common.h))
	@mkdir -p include/common
	@ln -sf $(CURDIR)/$(CHIP)/common/mhal_common.h include/common/mhal_common.h
endif
	$(MAKE) $(TARGET_ALL)
	$(MAKE) module DEBUG=$(DEBUG) 

module: $(foreach n,$(patsubst %_simple,%,$(TARGET_SIMPLE)),$(patsubst %,$(n)/%,$($(n)_COMPILE_NAME))) $(foreach n,$(patsubst %_raw-style,%,$(TARGET_RAW)) $(patsubst %_hal-style,%,$(TARGET_HAL)), $(n)/drv/lib.a $(n)/hal/lib.a)
	@ln -sf $(CURDIR)/kbuild.mk $(CHIP)/Makefile
	$(MAKE) -C $(CHIP) module DEBUG=$(DEBUG) HAL_LIBS="$(foreach n,$(patsubst %_simple,%,$(TARGET_SIMPLE)),$(patsubst %,$(n)/%,$($(n)_COMPILE_NAME))) $(foreach n,$(patsubst %_raw-style,%,$(TARGET_RAW)) $(patsubst %_hal-style,%,$(TARGET_HAL)), $(n)/drv/lib.a $(n)/hal/lib.a)"
	@rm -f $(CHIP)/Makefile

clean:
	$(MAKE) $(CLEAN_TARGET)
	@ln -sf $(CURDIR)/kbuild.mk $(CHIP)/Makefile
	@rm -rvf $(foreach n, Makefile $(foreach o,mhal.ko *.o mhal.mod.o,$(o) .$(o).cmd) mhal.mod.c modules.order Module.symvers .tmp_versions, $(CHIP)/$(n)) license.o .license.o.cmd
ifeq ($(CHIP)/utpa, $(wildcard $(CHIP)/utpa))
	$(MAKE) -C $(CHIP)/utpa clean
endif
ifeq (include/common/mhal_common.h, $(wildcard include/common/mhal_common.h))
	@rm -rvf include/common
endif

install:
	@cp -f $(CHIP)/mhal.ko $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)

$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config:
	$(MAKE) -C $(PROJ_ROOT) kbuild/$(KERNEL_VERSION)/.config

nop:
	@#nop
