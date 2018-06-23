############################################################ source files.
SUBDIRS:=.

EXEFILE    := service
OUTPUT_DIR := $(DB_OUT_PATH)/app
INC	 +=./inc
LIBS += -lMdbService

############################################################ depnedent header files.

include $(DB_BUILD_TOP)/app/Libs.mk
include $(DB_BUILD_TOP)/app/mdb/ModuleLibs.mk
include $(DB_BUILD_TOP)/compile.mk

all: gen_exe
clean: clean_files