include ../../../project/configs/current.configs
include $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk

SRCS:=$(wildcard src/*.c)

OBJS:=$(patsubst src/%.c,obj/%.o,$(SRCS))

all: $(OBJS)
	$(CC) -o config_tool $^

install:
	cp config_tool $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/config_tool

clean:
	rm -rf obj config_tool

obj/%.o:src/%.c obj
	$(CC) -c -o $@ $< -Iinc -I$(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME) -I$(PROJ_ROOT)/../sdk/interface/include/sys -I$(PROJ_ROOT)/../sdk/interface/include/common -Wall -g

obj:
	mkdir -p obj
