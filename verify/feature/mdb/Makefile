# All rights reserved.
#
# Makefile used for building application.
#
################################################################################
.PHONY : all clean lib app

include $(DB_ALKAID_PROJ)
export PROJ_ROOT PRODUCT CHIP BOARD TOOLCHAIN TOOLCHAIN_VERSION KERNEL_VERSION CUSTOMER_OPTIONS MOD_PREFIX INTERFACE_ENABLED INTERFACE_DISABLED MHAL_ENABLED MHAL_DISABLED

INTERFACE_MAKEFILE=./interface/

all: app

lib:
ifneq ($(DB_OUT_PATH), )
ifeq (./internal/Makefile, $(wildcard ./internal/Makefile))
	$(DB_MAKE) -C ./internal/ all
endif
ifeq ($(INTERFACE_MAKEFILE)/Makefile, $(wildcard $(INTERFACE_MAKEFILE)/Makefile))
	$(DB_MAKE) -C $(INTERFACE_MAKEFILE) all
endif
	$(DB_MAKE) -f ./MakeLib.mk all
else
	@echo "Please source xxx.sh first!"
endif

app: lib
ifneq ($(DB_OUT_PATH), )
	$(DB_MAKE) -C ./app/ all
else
	@echo "Please source xxx.sh first!"
endif

clean:
ifneq ($(DB_OUT_PATH), )
	$(DB_MAKE) -C ./app/ clean
ifeq ($(INTERFACE_MAKEFILE)/Makefile, $(wildcard $(INTERFACE_MAKEFILE)/Makefile))
	$(DB_MAKE) -C $(INTERFACE_MAKEFILE) clean
endif
ifeq (./internal/Makefile, $(wildcard ./internal/Makefile))
	$(DB_MAKE) -C ./internal/ clean
endif
	@rm -rvf ./out -rvf
else
	@echo "Please source xxx.sh first!"
endif

