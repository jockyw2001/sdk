LIBS     += -Wl,-rpath=./
LIBS     += -L$(DB_OUT_PATH)/lib -L$(DB_BUILD_TOP)/internal/src/base/rtsp/lib
LIBS     += -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/static
LIBS     += -l$(DB_LIB_NAME) -lmi_rgn -lmi_vpe -lmi_venc -lmi_vif -lmi_common -lmi_sys -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment
