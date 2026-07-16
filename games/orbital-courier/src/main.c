#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "rf_swan.h"
#include "native_art.h"

static const char __far title[] = "ORBITAL COURIER";
static const char __far subtitle[] = "Parcel 7: ring habitat";
static const char __far help[] = "D-pad move   A restart";
static const char __far fmt_status[] = "F%u S%u %s\n";
static const char __far outbound[] = "GET P";
static const char __far delivering[] = "TAKE P TO D";
static const char __far won[] = "DELIVERED! Press A again.";
static const char __far lost[] = "OUT OF FUEL. Press A.";

static bool blocked(uint8_t x, uint8_t y) {
	if (x == 0 || x == 19 || y == 0 || y == 8) return true;
	if (y == 3 && x > 3 && x < 15 && x != 9) return true;
	if (x == 12 && y > 3 && y < 8 && y != 6) return true;
	return false;
}

static void render(uint8_t px, uint8_t py, bool parcel, uint8_t fuel,
	uint8_t steps, uint8_t result) {
	uint8_t y;
	uint8_t x;
	rf_clear();
	rf_header(title, subtitle);
	printf(fmt_status, fuel, steps, parcel ? delivering : outbound);
	rf_playfield_begin();
	for (y = 0; y < 9; ++y) {
		for (x = 0; x < 20; ++x) {
			char c = blocked(x, y) ? '#' : '.';
			if (!parcel && x == 3 && y == 7) c = 'P';
			if (x == 17 && y == 1) c = 'D';
			if (x == px && y == py) c = '@';
			putchar(c);
		}
		putchar('\n');
	}
	rf_playfield_end();
	if (result == 1) printf(won);
	else if (result == 2) printf(lost);
	else printf("Pick up, route, deliver.\n");
	putchar('\n');
	rf_footer(help);
}

void main(void) {
	uint8_t px = 2;
	uint8_t py = 1;
	uint8_t fuel = 40;
	uint8_t steps = 0;
	uint8_t result = 0;
	bool parcel = false;
	bool dirty = true;

	rf_init(false);
	RF_LOAD_NATIVE_ART();
	while (1) {
		const rf_input_t *input;
		int8_t dx;
		int8_t dy;
		rf_frame();
		input = rf_input();

		if (result && (input->pressed & WS_KEY_A)) {
			px = 2; py = 1; fuel = 40; steps = 0; result = 0; parcel = false;
			dirty = true;
		}
		if (!result) {
			dx = rf_dx(input->pressed);
			dy = rf_dy(input->pressed);
			if (dx || dy) {
				uint8_t nx = (uint8_t)(px + dx);
				uint8_t ny = (uint8_t)(py + dy);
				if (!blocked(nx, ny)) {
					px = nx; py = ny; ++steps;
					if (fuel) --fuel;
					rf_beep(330, 2);
				} else {
					rf_beep(120, 4);
				}
				if (px == 3 && py == 7) parcel = true;
				if (parcel && px == 17 && py == 1) {
					result = 1;
					rf_beep(660, 12);
				} else if (fuel == 0) {
					result = 2;
				}
				dirty = true;
			}
		}
		if (dirty) {
			render(px, py, parcel, fuel, steps, result);
			dirty = false;
		}
	}
}
