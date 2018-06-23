M ?= $(CURDIR)

KDIR?=$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)
ifneq ($(HAL_LIBS),)
ifneq ($(CHIP),k6l)
KBUILD_EXTRA_SYMBOLS:=$(M)/utpa/Module.symvers
endif
EXTRA_LDFLAGS += --whole-archive
MODULE_NAME?=mhal
obj-m   := $(MODULE_NAME).o
$(MODULE_NAME)-objs+= $(patsubst $(CHIP)/%,%,$(filter $(CHIP)/%, $(HAL_LIBS))) $(patsubst %,../%,$(filter-out $(CHIP)/%, $(HAL_LIBS))) ../license.o module.o
else
MSTAR_INC_DIR:=$(INCS) include/mstar
MSTAR_INC_DIR+=drivers/mstar/include
EXTRA_CFLAGS += $(foreach n,$(MACROS),-D$(n)=1) $(foreach n,$(MSTAR_INC_DIR),-I$(n)) \
$(foreach n,$(MHAL_ENABLED),-DMHAL_$(shell tr 'a-z' 'A-Z' <<< $(n))=1) $(foreach n,$(MHAL_DISABLED),-DMHAL_$(shell tr 'a-z' 'A-Z' <<< $(n))=0)

ifeq ($(PROJ_ROOT)/../sdk/mhal/$(CHIP)/utpa, $(wildcard $(PROJ_ROOT)/../sdk/mhal/$(CHIP)/utpa$))
MSTAR_INC_DIR+=$(PROJ_ROOT)/../sdk/mhal/include/utopia
EXTRA_CFLAGS += -I$(PROJ_ROOT)/../sdk/mhal/include/utopia -include utopia_macros.h
endif

lib-y	:= $(patsubst %.c,%.o,$(SRCS))
endif
MSTAR_REMOVE_INCS:=all-exception

DEBUG ?= 0
ifeq ($(DEBUG), 1)
EXTRA_CFLAGS += -DDEBUG
endif

.PHONY:

module:
	@echo DEBUG=$(DEBUG)
	@echo EXTRA_CFLAGS=$(EXTRA_CFLAGS)
	@echo M=$(M)
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

lib:
	$(MAKE) -C $(KDIR) M=$(CURDIR)

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean

distclean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) distclean
