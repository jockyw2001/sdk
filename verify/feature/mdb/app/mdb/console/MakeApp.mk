############################################################ source files.
SUBDIRS:=.

EXEFILE    := console
OUTPUT_DIR := $(DB_OUT_PATH)/app
INC	 +=./inc
LIBS += -L$(DB_OUT_PATH)/lib
LIBS += -lMdbConsole

############################################################ depnedent header files.
include $(DB_BUILD_TOP)/app/Libs.mk
include $(DB_BUILD_TOP)/compile.mk

all: gen_exe
clean: clean_files