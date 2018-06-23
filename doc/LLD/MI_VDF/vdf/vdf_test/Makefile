TARGET_ROOT=$(shell pwd)

CROSS_COMPILE=arm-none-linux-gnueabi-

CC  = $(CROSS_COMPILE)gcc
AR  = $(CROSS_COMPILE)ar
LD  = $(CROSS_COMPILE)ld
STRIP  = $(CROSS_COMPILE)strip


TARGET:= vdf_test_k6l

#Add source file
SRCS := vdf_test_k6l.c  

COBJS := $(SRCS:.c=.o)

VDF_LIB_DIR := $(TARGET_ROOT)/../lib/
MI_LIB_DIR := $(TARGET_ROOT)/../MI/lib/
MI_SHADOW_LIB_DIR := $(TARGET_ROOT)/../MI/lib/
MI_COMMON_LIB_DIR := $(TARGET_ROOT)/../MI/lib/
MI_I3_LIB_DIR := $(TARGET_ROOT)/../MI/lib/
IE_LIB_DIR := $(TARGET_ROOT)/../IE/lib/
ISP_LIB_DIR := $(TARGET_ROOT)/../ISP/lib/

LIB_PATH := $(VDF_LIB_DIR)

LIB_SRC := \
	-lmi_vdf  

IE_LIB_SRC := -lMTE_LINUX
MI_SHADOW_LIB_SRC := -lmi_shadow
MI_COMMON_LIB_SRC := -lmi_common

LIB += $(foreach lib,$(IE_LIB_SRC),$(addprefix -L$(IE_LIB_DIR) ,$(lib)))
LIB += $(foreach lib,$(MI_LIB_SRC),$(addprefix -L$(MI_LIB_DIR) ,$(lib)))
LIB += $(foreach lib,$(MI_SHADOW_LIB_SRC),$(addprefix -L$(MI_SHADOW_LIB_DIR) ,$(lib)))
LIB += $(foreach lib,$(MI_COMMON_LIB_SRC),$(addprefix -L$(MI_COMMON_LIB_DIR) ,$(lib)))
LIB += $(foreach lib,$(LIB_SRC),$(addprefix -L$(LIB_PATH) ,$(lib)))

LDFLAGS += \
     -lpthread \
     -lrt \
     -ldl \
     -lc  \
     -lm 

#Add include files
INCDIR := $(VDF_LIB_DIR)/../ 

INCDIR := $(foreach incdir,$(INCDIR),$(addprefix -I,$(incdir)))


#Add gcc compile flag
CFLAGS  = -Wall -Os
CFLAGS += -fPIE

all:
	@echo "make $(TARGET_NAME)"
	@$(MAKE) $(COBJS)
	@$(CC) $(COBJS) $(CFLAGS) $(INCDIR) $(LIB) $(LDFLAGS) -o $(TARGET)
	$(STRIP) -s $(TARGET)

clean:
	@$(shell if [ -f $(TARGET) ]; then rm $(TARGET); fi;)
	rm *.o

$(COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCDIR) -c $< -o $@
	
