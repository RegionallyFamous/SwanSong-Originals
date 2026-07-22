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

.PHONY: all art clean dist verify native-test test quality quality-strict smoke playtester-check $(GAMES)

all: $(GAMES)

art:
	python3 tools/build_fullscreen_art.py

$(GAMES):
	$(MAKE) -C games/$@

dist: all
	@mkdir -p dist
	@for game in $(GAMES); do \
		cp games/$$game/*.wsc dist/; \
	done

verify: dist
	python3 tools/verify_roms.py dist/*.wsc

native-test:
	$(MAKE) -C tests/native

test: verify native-test
	python3 tools/test_game_invariants.py
	python3 tools/test_gameplay_paths.py
	python3 tools/test_native_art.py
	python3 tools/test_sdk_manifests.py

quality:
	python3 tools/test_game_quality.py --mode report

quality-strict:
	@test -n "$(GAME)" || { echo "GAME=<game-slug> is required" >&2; exit 2; }
	python3 tools/test_game_quality.py --mode strict --game "$(GAME)"

smoke: test
	tools/smoke_swansong.sh dist/*.wsc

playtester-check: dist
	plugins/swansong-playtester/tests/check_swansong_bridge.sh

clean:
	$(MAKE) -C tests/native clean
	@for game in $(GAMES); do \
		$(MAKE) -C games/$$game clean; \
	done
	rm -rf dist
