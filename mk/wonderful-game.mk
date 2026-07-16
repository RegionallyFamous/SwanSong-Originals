WONDERFUL_TOOLCHAIN ?= /opt/wonderful
TARGET := wswan/medium
include $(WONDERFUL_TOOLCHAIN)/target/$(TARGET)/makedefs.mk

INCLUDEDIRS := include ../../engine/include
SOURCEDIRS := src
LIBS := -lwse -lwsx -lws
LIBDIRS := $(WF_ARCH_LIBDIRS)
BUILDDIR := build
ELF := build/$(NAME).elf
ELF_STAGE1 := build/$(NAME)_stage1.elf
ROM := $(NAME).wsc
ENGINE_LIB := ../../engine/build/librf_swan.a
ENGINE_INPUTS := $(shell find -L ../../engine/include ../../engine/src -type f) ../../engine/Makefile

SOURCES_C := $(shell find -L $(SOURCEDIRS) -name "*.c")
OBJS := $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_C)))
DEPS := $(OBJS:.o=.d)

INCLUDEFLAGS := $(foreach path,$(INCLUDEDIRS),-I$(path)) \
	$(foreach path,$(LIBDIRS),-isystem $(path)/include)
LIBDIRSFLAGS := $(foreach path,$(LIBDIRS),-L$(path)/lib)
CFLAGS += -std=gnu11 -Wall -Wextra -Werror $(WF_ARCH_CFLAGS) \
	$(INCLUDEFLAGS) -ffunction-sections -fdata-sections -fno-common -O2
LDFLAGS := -T$(WF_LDSCRIPT) $(LIBDIRSFLAGS) $(WF_ARCH_LDFLAGS) $(LIBS)

.PHONY: all clean usage

all: $(ROM) compile_commands.json

$(ENGINE_LIB): $(ENGINE_INPUTS)
	$(MAKE) -C ../../engine

$(ROM) $(ELF): $(ELF_STAGE1) wfconfig.toml
	@echo "  ROM     $@"
	$(BUILDROM) -o $(ROM) --output-elf $(ELF) $<

$(ELF_STAGE1): $(OBJS) $(ENGINE_LIB)
	@echo "  LD      $@"
	$(CC) -r -o $@ $(OBJS) $(ENGINE_LIB) $(WF_CRT0) $(LDFLAGS)

$(BUILDDIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -MJ $(@:.o=.cc.json) -c -o $@ $<

compile_commands.json: $(OBJS)
	$(WF)/bin/wf-compile-commands-merge $@ $(patsubst %.o,%.cc.json,$^)

usage: $(ELF)
	$(ROMUSAGE) $< -g

clean:
	rm -rf $(ROM) $(BUILDDIR) compile_commands.json

-include $(DEPS)
