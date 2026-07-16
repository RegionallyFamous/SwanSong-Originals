WONDERFUL_TOOLCHAIN ?= /opt/wonderful
TARGET := wswan/medium
include $(WONDERFUL_TOOLCHAIN)/target/$(TARGET)/makedefs.mk

INCLUDEDIRS := include
SOURCES := $(wildcard src/*.c)
OBJECTS := $(patsubst src/%.c,build/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)
LIB := build/librf_swan.a

INCLUDEFLAGS := $(foreach path,$(INCLUDEDIRS),-I$(path)) \
	$(foreach path,$(WF_ARCH_LIBDIRS),-isystem $(path)/include)
CFLAGS += -std=gnu11 -Wall -Wextra -Werror $(WF_ARCH_CFLAGS) \
	$(INCLUDEFLAGS) -ffunction-sections -fdata-sections -fno-common -O2

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJECTS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^

build/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

clean:
	rm -rf build

-include $(DEPS)
