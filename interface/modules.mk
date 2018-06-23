M ?= $(CURDIR)

include $(PROJ_ROOT)/release/customer_options/$(CUSTOMER_OPTIONS)
include $(PROJ_ROOT)/../sdk/interface/compile_options.mk

KDIR?=$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)

MSTAR_INC_DIR:= $(foreach n,$(MODULE_DIR) $(DEP_MODULE) $(DEP_EXT_INC), $(PROJ_ROOT)/../sdk/interface/include/$(n)) \
        $(PROJ_ROOT)/../sdk/impl/$(MODULE_DIR) \
        $(PROJ_ROOT)/../sdk/interface/include/internal/kernel \
        $(foreach n,$(DEP_HAL_INC),$(PROJ_ROOT)/../sdk/mhal/include/$(n))

EXTRA_CFLAGS:= -g -Werror -Wall -Wno-unused-result \
			-DMSTAR_MODULE_VERSION="Sigmastar Module version: project_commit.$(shell git log -n 1 --format="%h") sdk_commit.$(shell cd $(PROJ_ROOT)/../sdk; git log -n 1 --format="%h";)$(shell cd -;) build_time.$(shell date +%Y%m%d%H%M%S)" \
			-DEXTRA_MODULE_NAME=$(MODULE_DIR) $(EXTRA_INCS) \
			-DMI_DBG=$(MI_DBG) \
			-DSUPPORT_HDMI_VGA_DIRECT_MODE=${SUPPORT_HDMI_VGA_DIRECT_MODE} \
			-DSUPPORT_DIVP_USE_GE_SCALING_UP=${SUPPORT_DIVP_USE_GE_SCALING_UP} \
			-DSUPPORT_VIF_USE_GE_FILL_BUF=${SUPPORT_VIF_USE_GE_FILL_BUF} \
			-DSUPPORT_MsOS_MPool_Add_PA2VARange=${SUPPORT_MsOS_MPool_Add_PA2VARange} \
			-DSUPPORT_DISP_ALIGN_UP_OFFSET32=${SUPPORT_DISP_ALIGN_UP_OFFSET32} \
			$(foreach n,$(MSTAR_INC_DIR),-I$(n)) $(foreach n,$(INTERFACE_ENABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n))=1) $(foreach n,$(INTERFACE_DISABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n))=0) $(foreach n,$(MHAL_ENABLED),-DMHAL_$(shell tr 'a-z' 'A-Z' <<< $(n))=1) $(foreach n,$(MHAL_DISABLED),-DMHAL_$(shell tr 'a-z' 'A-Z' <<< $(n))=0)

ifeq ($(CHIP), i2)
EXTRA_CFLAGS+= -DSUPPORT_VDEC_MULTI_RES=1
EXTRA_CFLAGS+= -DCONFIG_MSTAR_CHIP_I2=1
else ifeq ($(CHIP), k6)
EXTRA_CFLAGS+= -DCONFIG_MSTAR_CHIP_K6=1
else ifeq ($(CHIP), k6l)
EXTRA_CFLAGS+= -DCONFIG_MSTAR_CHIP_K6LITE=1
endif

KBUILD_EXTRA_SYMBOLS:=$(foreach m, $(DEP_MODULE), $(M)/../$(m)/Module.symvers) \
                      $(PROJ_ROOT)/../sdk/mhal/$(CHIP)/Module.symvers


ifeq ($(KERNEL_VERSION), 3.18.30)
EXTRA_CFLAGS+= -I$(KDIR)/drivers/mstar/include/ -I$(KDIR)/drivers/mstar/cpu/include/ -I$(KDIR)/drivers/mstar/miu/

#$(KDIR)/include $(KDIR)/drivers/mstar/include $(KDIR)/drivers/mstar/cpu/include/ $(KDIR)/drivers/mstar/miu/ \
#$(KDIR)/drivers/mstar/mma_heap/ $(KDIR)/mm \

else
EXTRA_CFLAGS+= -I$(KDIR)/mstar2/include/
endif

ifeq ($(PROJ_ROOT)/../sdk/mhal/$(CHIP)/utpa, $(wildcard $(PROJ_ROOT)/../sdk/mhal/$(CHIP)/utpa$))
MSTAR_INC_DIR+=$(PROJ_ROOT)/../sdk/mhal/include/utopia
EXTRA_CFLAGS += -I$(PROJ_ROOT)/../sdk/mhal/include/utopia -include utopia_macros.h
endif

MI_MODULE_NAME:=$(MOD_PREFIX)$(MODULE_DIR)

obj-m	:= $(MI_MODULE_NAME).o
ifeq ($(KAPI_DISABLE),)
$(MI_MODULE_NAME)-objs+= $(patsubst %.c, %.o, $(API_FILE))
endif
#ifeq ($(MODULE_DIR),common)
MSTAR_REMOVE_INCS:=all-exception
$(MI_MODULE_NAME)-objs+= $(patsubst %.c, %.o, $(KINTERNAL_FILES))
#else
ifneq ($(API_FILE),)
$(API_FILE) = exception
endif
ifneq ($(WRAPPER_FILE),)
$(WRAPPER_FILE) = exception
endif
#endif
$(MI_MODULE_NAME)-objs+= $(patsubst %.c, %.o, $(WRAPPER_FILE))
$(MI_MODULE_NAME)-objs+= $(patsubst %.c, ../../../impl/$(MODULE_DIR)/%.o, $(IMPL_FILE))
$(MI_MODULE_NAME)-objs+= $(foreach h,$(DEP_HAL),../../../hal/$(HAL)/$(h)/lib.a)

THIS_KO:=$(M)/$(MI_MODULE_NAME).ko

ifeq ($(filter $(MI_MODULE_NAME).c, $(SOURCE_FILE)),)
module:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules MI_MODULE_NAME=$(MI_MODULE_NAME)
else
module:
	@echo rename source file $(MI_MODULE_NAME).c
	@exit;
endif

module_install:
ifneq ($(WRAPPER_FILE),)
	cp -f $(THIS_KO) $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)
endif

module_clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
	rm -f $(foreach n, $(patsubst %.c, %.o, $(IMPL_FILE)), ../../../impl/$(MODULE_DIR)/$(n) ../../../impl/$(MODULE_DIR)/$(dir $(n)).$(notdir $(n)).cmd)

ifeq ($(UAPI_DISABLE),)
API_OBJ:=$(patsubst %.c, %_obj.o, $(API_FILE) $(UINTERNAL_FILES))
endif

ifneq ($(API_OBJ),)
lib: $(API_OBJ)
	$(CC) -shared -o lib$(MI_MODULE_NAME).so $(API_OBJ) $(LOCAL_LDFLAGS)
	$(AR) csq lib$(MI_MODULE_NAME).a $(API_OBJ)

$(API_OBJ):%_obj.o: %.c
	@$(CC) -o $@ -c $^ -g -fPIC -DEXTRA_MODULE_NAME=$(MODULE_DIR) -DMI_DBG=$(MI_DBG) $(foreach n,$(MODULE_DIR) $(DEP_EXT_INC) $(DEP_MODULE), -I$(PROJ_ROOT)/../sdk/interface/include/$(n)) -I$(PROJ_ROOT)/../sdk/interface/include/internal/user $(foreach n,$(INTERFACE_ENABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n))=1) $(foreach n,$(INTERFACE_DISABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n))=0) $(foreach n,$(MHAL_ENABLED),-DMHAL_$(shell tr 'a-z' 'A-Z' <<< $(n))=1) $(foreach n,$(MHAL_DISABLED),-DMHAL_$(shell tr 'a-z' 'A-Z' <<< $(n))=0)
else
lib:
	@#
endif

lib_install:
ifeq ($(UAPI_DISABLE),)
	cp -f $(foreach d,$(MODULE_DIR) $(DEP_EXT_INC), $(PROJ_ROOT)/../sdk/interface/include/$(d)/$(MOD_PREFIX)$(d).h) $(TARGET_INCLUDEDIR)
	cp -f $(foreach d,$(MODULE_DIR) $(DEP_EXT_INC), $(PROJ_ROOT)/../sdk/interface/include/$(d)/$(MOD_PREFIX)$(d)_datatype.h) $(TARGET_INCLUDEDIR)
endif
ifneq ($(API_OBJ),)
	cp lib$(MI_MODULE_NAME).so $(TARGET_LIBDIR)/dynamic
	cp lib$(MI_MODULE_NAME).a $(TARGET_LIBDIR)/static
endif

lib_clean:
	rm -f $(API_OBJ) lib$(MI_MODULE_NAME).so lib$(MI_MODULE_NAME).a
