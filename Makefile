GAMES := \
	mote-sound-terminal \
	orbital-courier \
	scrapframe-garage \
	radio-ghost \
	harpoon-moon \
	turncoat-tactics \
	pocket-kaiju-observatory \
	rotate-dungeon \
	one-last-lap \
	bug-witch

.PHONY: all art clean dist engine verify test smoke playtester-check $(GAMES)

all: engine $(GAMES)

art:
	python3 tools/build_fullscreen_art.py

engine:
	$(MAKE) -C engine

$(GAMES): engine
	$(MAKE) -C games/$@

dist: all
	@mkdir -p dist
	@for game in $(GAMES); do \
		cp games/$$game/*.wsc dist/; \
	done

verify: dist
	python3 tools/verify_roms.py dist/*.wsc

test: verify
	python3 tools/test_game_invariants.py
	python3 tools/test_gameplay_paths.py
	python3 tools/test_native_art.py

smoke: test
	tools/smoke_swansong.sh dist/*.wsc

playtester-check: dist
	plugins/swansong-playtester/tests/check_swansong_bridge.sh

clean:
	$(MAKE) -C engine clean
	@for game in $(GAMES); do \
		$(MAKE) -C games/$$game clean; \
	done
	rm -rf dist
