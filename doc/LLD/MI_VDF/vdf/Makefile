CROSS_COMPILE=arm-none-linux-gnueabi-
GCC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
AR = $(CROSS_COMPILE)ar

PROGS = libmi_vdf
VDF_TOP = $(shell pwd)
OBJDIR  = $(VDF_TOP)/obj
LIBDIR  = $(VDF_TOP)/lib

IE_INCLUDE = $(VDF_TOP)/IE/inc/
MI_INCLUDE = $(VDF_TOP)/MI/inc/
#MI_I3_INCLUDE = $(VDF_TOP)/MI_I3/inc/


C_INCLUDES := \
	-I./inc \
	-I$(IE_INCLUDE) \
	-I$(MI_INCLUDE) \
	-I$(MI_INCLUDE_I3) \
	-I$(VDF_TOP)/../../include/common/ \

#C_FLAGS := -Wall -Werror -O2 -mcpu=cortex-a7 -fPIC -DPIC -g -ffunction-sections -funwind-tables -fstack-protector -DDMALLOC -mfpu=neon
C_FLAGS := -O2 -mcpu=cortex-a7 -fPIC -DPIC -g -ffunction-sections -funwind-tables -fstack-protector -DDMALLOC -mfpu=neon
C_FLAGS_STATIC := -Wall -O3 -mcpu=cortex-a7 -ffunction-sections -funwind-tables -fstack-protector -DDMALLOC -mfpu=neon
#C_FLAGS_STATIC += -DCONFIG_DEBUG_LEVEL=255

LDFLAGS := -O2 -Bdirect -Wl,--hash-style=gnu -ldl -lpthread
LDFLAGS += -lgcc_s -rdynamic


IE_LIBS := $(VDF_TOP)/../IE/lib/libMTE_LINUX.so 



MI_COMP_C_SRCS=$(wildcard ./*.c)
MI_COMP_C_SRCS_NO_DIR=$(notdir $(MI_COMP_C_SRCS))
OBJECTS :=$(patsubst %.c, %.so.o,  $(MI_COMP_C_SRCS_NO_DIR))

OBJPROG := $(addprefix $(OBJDIR)/, $(PROGS).so)
OBJPROG_STATIC = $(addprefix $(OBJDIR)/, $(PROGS).a)

.PHONY: clean install

all: $(OBJPROG)   $(OBJPROG_STATIC)
	

#generate libmi_vdf.so
$(OBJPROG):	$(addprefix $(OBJDIR)/, $(OBJECTS))
	mkdir -p ${OBJDIR}
	mkdir -p ${LIBDIR}
	@echo "  LIB $@"
	$(GCC) -shared $(LDFLAGS) -o $@ $(addprefix $(OBJDIR)/, $(OBJECTS)) $(OBJ_LIB)
	cp -f ${OBJDIR}/*.so ${LIBDIR}
	@echo ""

$(OBJDIR)/%.so.o : ./%.c
	@mkdir -p obj
	@echo "  CC  $<"
	@$(GCC) $(C_FLAGS) $(C_INCLUDES) -c $< -o $@

#generate libmi_vdf.a
OBJECTS_STATIC :=$(patsubst %.c, %.a.o,  $(MI_COMP_C_SRCS_NO_DIR))

$(OBJPROG_STATIC): $(addprefix $(OBJDIR)/, $(OBJECTS_STATIC))
	@echo " LIB $@"
	$(AR) -crD  $@ $(addprefix $(OBJDIR)/, $(OBJECTS_STATIC)) $(OBJ_LIB)
	cp -f ${OBJDIR}/*.a ${LIBDIR}
	@echo ""

$(OBJDIR)/%.a.o : ./%.c
	@echo "  CC  $<"
	@$(GCC) $(C_FLAGS_STATIC) $(C_INCLUDES) -c $< -o $@


clean:
	@rm -Rf $(OBJDIR)
	@rm -Rf $(destlib)
	@rm -Rf $(LIBDIR)
