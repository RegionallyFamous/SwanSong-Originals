HOST_CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -O2

.PHONY: all clean

all: build/test_one_last_lap build/test_orbital_courier build/test_mote_sound_terminal \
	build/test_scrapframe_garage build/test_bug_witch build/test_radio_ghost \
	build/test_harpoon_moon build/test_pocket_kaiju build/test_rotate_dungeon \
	build/test_turncoat_tactics
	./build/test_one_last_lap
	./build/test_orbital_courier
	./build/test_mote_sound_terminal
	./build/test_scrapframe_garage
	./build/test_bug_witch
	./build/test_radio_ghost
	./build/test_harpoon_moon
	./build/test_pocket_kaiju
	./build/test_rotate_dungeon
	./build/test_turncoat_tactics

build/test_one_last_lap: test_one_last_lap.c ../../games/one-last-lap/src/model.c \
		../../games/one-last-lap/src/model.h
	@mkdir -p build
	$(HOST_CC) $(CFLAGS) -I../../games/one-last-lap/src \
		test_one_last_lap.c ../../games/one-last-lap/src/model.c -o $@

build/test_orbital_courier: test_orbital_courier.c \
		../../games/orbital-courier/src/model.c ../../games/orbital-courier/src/model.h
	@mkdir -p build
	$(HOST_CC) $(CFLAGS) -I../../games/orbital-courier/src \
		test_orbital_courier.c ../../games/orbital-courier/src/model.c -o $@

build/test_mote_sound_terminal: test_mote_sound_terminal.c \
		../../games/mote-sound-terminal/src/model.c \
		../../games/mote-sound-terminal/src/model.h
	@mkdir -p build
	$(HOST_CC) $(CFLAGS) -I../../games/mote-sound-terminal/src \
		test_mote_sound_terminal.c ../../games/mote-sound-terminal/src/model.c -o $@

build/test_scrapframe_garage: test_scrapframe_garage.c \
		../../games/scrapframe-garage/src/model.c \
		../../games/scrapframe-garage/src/model.h
	@mkdir -p build
	$(HOST_CC) $(CFLAGS) -I../../games/scrapframe-garage/src \
		test_scrapframe_garage.c ../../games/scrapframe-garage/src/model.c -o $@

build/test_bug_witch: test_bug_witch.c ../../games/bug-witch/src/model.c \
		../../games/bug-witch/src/model.h
	@mkdir -p build
	$(HOST_CC) $(CFLAGS) -I../../games/bug-witch/src \
		test_bug_witch.c ../../games/bug-witch/src/model.c -o $@

build/test_radio_ghost: test_radio_ghost.c ../../games/radio-ghost/src/model.c \
		../../games/radio-ghost/src/model.h
	@mkdir -p build
	$(HOST_CC) $(CFLAGS) -I../../games/radio-ghost/src \
		test_radio_ghost.c ../../games/radio-ghost/src/model.c -o $@

build/test_harpoon_moon: test_harpoon_moon.c ../../games/harpoon-moon/src/model.c \
		../../games/harpoon-moon/src/model.h
	@mkdir -p build
	$(HOST_CC) $(CFLAGS) -I../../games/harpoon-moon/src \
		test_harpoon_moon.c ../../games/harpoon-moon/src/model.c -o $@

build/test_pocket_kaiju: test_pocket_kaiju.c \
		../../games/pocket-kaiju-observatory/src/model.c \
		../../games/pocket-kaiju-observatory/src/model.h
	@mkdir -p build
	$(HOST_CC) $(CFLAGS) -I../../games/pocket-kaiju-observatory/src \
		test_pocket_kaiju.c ../../games/pocket-kaiju-observatory/src/model.c -o $@

build/test_rotate_dungeon: test_rotate_dungeon.c \
		../../games/rotate-dungeon/src/model.c ../../games/rotate-dungeon/src/model.h
	@mkdir -p build
	$(HOST_CC) $(CFLAGS) -I../../games/rotate-dungeon/src \
		test_rotate_dungeon.c ../../games/rotate-dungeon/src/model.c -o $@

build/test_turncoat_tactics: test_turncoat_tactics.c \
		../../games/turncoat-tactics/src/model.c ../../games/turncoat-tactics/src/model.h
	@mkdir -p build
	$(HOST_CC) $(CFLAGS) -I../../games/turncoat-tactics/src \
		test_turncoat_tactics.c ../../games/turncoat-tactics/src/model.c -o $@

clean:
	rm -rf build
