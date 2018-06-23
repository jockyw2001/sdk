include $(CURDIR)/../../project/configs/current.configs

include $(PROJ_ROOT)/release/customer_tailor/$(CUSTOMER_TAILOR)

MISC_MODULES:= fbdev config_tool

MISC_ENABLED:=$(patsubst %_enable_,%,$(patsubst %__,%,$(filter-out %_disable_, $(foreach n,$(MISC_MODULES),$(n)_$(misc_$(n))_))))
MISC_DISABLED:=$(filter-out $(INTERFACE_ENABLED),$(INTERFACE_MODULES))
MISC_MODULES_CLEAN := $(foreach n,$(MISC_ENABLED),$(n)_clean)
MISC_MODULES_INSTALL := $(foreach n,$(MISC_ENABLED),$(n)_install)

.PHONY: all clean install $(MISC_MODULES_CLEAN) $(MISC_MODULES_INSTALL) $(MISC_ENABLED)

all: $(MISC_ENABLED)
clean: $(MISC_MODULES_CLEAN)
install: $(MISC_MODULES_INSTALL)

$(MISC_ENABLED):
	$(MAKE) -C $@ all

$(MISC_MODULES_CLEAN):
	$(MAKE) -C $(patsubst %_clean,%,$@) clean

$(MISC_MODULES_INSTALL):
	$(MAKE) -C $(patsubst %_install,%,$@) install
