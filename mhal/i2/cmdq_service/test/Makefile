CROSS_COMPILE =arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip

LOCAL_MODULE_NAME = cmdqtest
LOCAL_CINCLUDES  = -I../drv/pub
CFLAGS 	  = -Wall -Werror -lpthread -ld
CINCLUDES = -I../drv/pub


LOCAL_CSRCS=$(wildcard ./*.c)
LOCAL_MODULE_FILE=$(LOCAL_MODULE_NAME)

LOCAL_COBJS = $(patsubst %.c, %.c.o,  $(LOCAL_CSRCS))

all: prepare $(LOCAL_MODULE_NAME) finish

clean:
	echo "  CLEAN $(shell pwd)"
	rm -Rf $(LOCAL_COBJS)
	rm -f  $(LOCAL_MODULE_NAME)

prepare:
	@echo
	@echo ">>>>========================================================"
	@echo
	@echo "  PWD = $(shell pwd)"
	@echo "  CC = $(CC)"
	@echo "  LOCAL_MODULE_FILE = $(LOCAL_MODULE_FILE)"
	@echo

finish:
	@echo "<<<<========================================================"
	@echo

$(LOCAL_MODULE_NAME):  $(LOCAL_COBJS)
	@mkdir -p $(LOCAL_BUILD_DIR)/bin
	@echo "  LD      $(LOCAL_BUILD_DIR)/bin/$@"
	$(CC) $(LDFLAGS) $(LOCAL_LDFLAGS) -o $@ $(LOCAL_COBJS) $(LIBS) $(LOCAL_LIBS)
	@echo

%.c.o : %.c
	@mkdir -p $(dir $@)
	echo "  CC      $<"
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(CINCLUDES) $(LOCAL_CINCLUDES)  -c $< -o $@