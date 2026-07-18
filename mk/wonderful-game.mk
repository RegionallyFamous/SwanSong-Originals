WONDERFUL_TOOLCHAIN ?= /opt/wonderful
SWANSONG_SDK_DIR ?= $(abspath ../../vendor/swansong-sdk)
SWAN ?= env PYTHONPATH=$(SWANSONG_SDK_DIR)/python python3 -m swansong_sdk.cli
SWAN_GFX_HARDWARE_TILE_CAPACITY ?= $(shell $(SWAN) hardware-tile-capacity --project swan.toml)
TARGET := wswan/medium
include $(WONDERFUL_TOOLCHAIN)/target/$(TARGET)/makedefs.mk

HOST_TEST_NAME ?= $(NAME)
HOST_TEST := ../../tests/native/build/test_$(HOST_TEST_NAME)

INCLUDEDIRS := include src ../../shared build/generated/include $(SWANSONG_SDK_DIR)/include
LIBDIRS := $(WF_ARCH_LIBDIRS)
BUILDDIR := build/obj
ELF := build/$(NAME).elf
ELF_STAGE1 := build/$(NAME)_stage1.elf
ROM := $(NAME).wsc
SWANSONG_RUNTIME_BUILD := $(abspath build/swansong-sdk-$(SWAN_GFX_HARDWARE_TILE_CAPACITY))
SWANSONG_RUNTIME := $(SWANSONG_RUNTIME_BUILD)/$(TARGET)/libswan.a
GENERATED_STAMP := build/generated/.stamp
GENERATED_ASSET_SOURCES_C ?=
GENERATED_SOURCES_C := build/generated/src/swan_assets.c \
	build/generated/src/swan_config.c $(GENERATED_ASSET_SOURCES_C)
GAME_SOURCES_C := $(wildcard src/*.c)
SHARED_SOURCES_C := ../../shared/swan_game_runtime.c
GAME_OBJS := $(addprefix $(BUILDDIR)/,$(addsuffix .o,$(GAME_SOURCES_C) $(GENERATED_SOURCES_C)))
SHARED_OBJS := $(BUILDDIR)/shared/swan_game_runtime.c.o
OBJS := $(GAME_OBJS) $(SHARED_OBJS)
DEPS := $(OBJS:.o=.d)
ASSET_INPUTS := swan.toml $(shell find assets tests/play -type f 2>/dev/null)
SDK_TOOL_INPUTS := $(shell find -L $(SWANSONG_SDK_DIR)/python/swansong_sdk -type f)
SDK_RUNTIME_INPUTS := $(shell find -L $(SWANSONG_SDK_DIR)/include \
	$(SWANSONG_SDK_DIR)/src -type f) $(SWANSONG_SDK_DIR)/mk/runtime-library.mk

INCLUDEFLAGS := $(foreach path,$(INCLUDEDIRS),-I$(path)) \
	$(foreach path,$(LIBDIRS),-isystem $(path)/include)
LIBDIRSFLAGS := $(foreach path,$(LIBDIRS),-L$(path)/lib)
CFLAGS += -std=gnu11 -Wall -Wextra -Werror \
	-DSWAN_GFX_HARDWARE_TILE_CAPACITY=$(SWAN_GFX_HARDWARE_TILE_CAPACITY) $(WF_ARCH_CFLAGS) \
	$(INCLUDEFLAGS) -ffunction-sections -fdata-sections -fno-common -O2
LDFLAGS := -T$(WF_LDSCRIPT) $(LIBDIRSFLAGS) $(WF_ARCH_LDFLAGS) \
	-Wl,--start-group -lwse -lwsx -lws -lc -Wl,--end-group

.PHONY: all assets clean sdk-runtime test usage

all: assets $(ROM) compile_commands.json

test: all
	$(MAKE) -C ../../tests/native build/test_$(HOST_TEST_NAME)
	$(HOST_TEST)

assets: $(GENERATED_STAMP)

$(GENERATED_STAMP): $(ASSET_INPUTS) $(SDK_TOOL_INPUTS)
	$(SWAN) assets --project swan.toml
	@touch $@

$(GENERATED_SOURCES_C): $(GENERATED_STAMP)
	@test -f $@

sdk-runtime: $(SWANSONG_RUNTIME)

$(SWANSONG_RUNTIME): $(SDK_RUNTIME_INPUTS)
	$(MAKE) -C $(SWANSONG_SDK_DIR) -f mk/runtime-library.mk \
		TARGET=$(TARGET) BUILD_ROOT=$(SWANSONG_RUNTIME_BUILD) \
		SWAN_GFX_HARDWARE_TILE_CAPACITY=$(SWAN_GFX_HARDWARE_TILE_CAPACITY) all

$(ROM) $(ELF): $(ELF_STAGE1) wfconfig.toml
	@echo "  ROM     $@"
	$(BUILDROM) -o $(ROM) --output-elf $(ELF) $<

$(ELF_STAGE1): $(OBJS) $(SWANSONG_RUNTIME)
	@mkdir -p $(@D)
	@echo "  LD      $@"
	$(CC) -r -o $@ $(OBJS) $(SWANSONG_RUNTIME) $(WF_CRT0) $(LDFLAGS)

$(BUILDDIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -MJ $(@:.o=.cc.json) -c -o $@ $<

$(BUILDDIR)/shared/%.c.o: ../../shared/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -MJ $(@:.o=.cc.json) -c -o $@ $<

compile_commands.json: $(OBJS)
	$(WF)/bin/wf-compile-commands-merge $@ $(patsubst %.o,%.cc.json,$^)

usage: $(ELF)
	$(ROMUSAGE) $< -g

clean:
	rm -rf $(ROM) $(BUILDDIR) compile_commands.json
	rm -rf build/generated build/swansong-sdk-* build/$(NAME).elf build/$(NAME)_stage1.elf

-include $(DEPS)
