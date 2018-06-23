
############################################################ source files.
SUBDIRS :=./src ./src/base/common/src ./src/base/rtsp/src
INC     +=./inc ./src/base/common/inc
INC     += $(PROJ_ROOT)/release/$(PRODUCT)/include
INC  += ./src/base/rtsp/include/UsageEnvironment
INC  += ./src/base/rtsp/include/groupsock
INC  += ./src/base/rtsp/include/liveMedia
INC  += ./src/base/rtsp/include/BasicUsageEnvironment
INC  += ./src/base/rtsp/src

OUTPUT_DIR := $(DB_OUT_PATH)/obj/

############################################################ depnedent header files.
include $(DB_BUILD_TOP)/compile.mk
all: gen_obj
clean: clean_files
