#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "rf_swan.h"

static const char __far title[] = "ROTATE DUNGEON";
static const char __far subtitle[] = "The room changes when turned";
static const char __far help[] = "Move  START rotate  B reset";
static const char __far fmt_status[] = "ROOM %u/5  KEY %s  VIEW %s\n";
static const char __far yes_text[] = "YES";
static const char __far no_text[] = "NO";
static const char __far horiz_text[] = "H";
static const char __far vert_text[] = "V";

static bool blocked(uint8_t room, bool vertical, uint8_t x, uint8_t y) {
	uint8_t gap;
	if (x == 0 || x == 11 || y == 0 || y == 7) return true;
	if (!vertical) {
		gap = (uint8_t)(1 + room % 6);
		return x == (uint8_t)(3 + room % 5) && y != gap;
	}
	gap = (uint8_t)(1 + (room * 2) % 10);
	return y == (uint8_t)(2 + room % 4) && x != gap;
}

static uint8_t key_x(uint8_t room) { return (uint8_t)(2 + room); }
static uint8_t key_y(uint8_t room) { return (uint8_t)(6 - (room & 1)); }

static void render(uint8_t room, bool vertical, uint8_t px, uint8_t py,
	bool key, uint8_t result) {
	uint8_t y;
	uint8_t x;
	rf_clear();
	rf_header(title, subtitle);
	printf(fmt_status, room + 1, key ? yes_text : no_text, vertical ? vert_text : horiz_text);
	for (y = 0; y < 8; ++y) {
		for (x = 0; x < 12; ++x) {
			char c = blocked(room, vertical, x, y) ? '#' : '.';
			if (!key && x == key_x(room) && y == key_y(room)) c = 'K';
			if (x == 10 && y == 1) c = 'E';
			if (x == px && y == py) c = '@';
			putchar(c);
		}
		putchar('\n');
	}
	printf("Rotate changes solid walls.\n");
	if (result) printf("FLOOR CLEARED! A restart\n");
	else printf("Find K, then reach E.\n");
	rf_footer(help);
}

void main(void) {
	uint8_t room = 0;
	uint8_t px = 1;
	uint8_t py = 1;
	uint8_t result = 0;
	bool vertical = false;
	bool key = false;
	bool dirty = true;

	rf_init(false);
	while (1) {
		const rf_input_t *input;
		rf_frame();
		input = rf_input();

		if (result && (input->pressed & WS_KEY_A)) {
			room = 0; px = 1; py = 1; result = 0; vertical = false; key = false;
			rf_set_orientation(false); dirty = true;
		}
		if (!result) {
			int8_t dx = rf_dx(input->pressed);
			int8_t dy = rf_dy(input->pressed);
			if (input->pressed & WS_KEY_START) {
				vertical = !vertical;
				rf_set_orientation(vertical);
				if (blocked(room, vertical, px, py)) { px = 1; py = 1; }
				rf_beep(vertical ? 520 : 360, 6);
				dirty = true;
			}
			if (input->pressed & WS_KEY_B) {
				px = 1; py = 1; key = false; vertical = false;
				rf_set_orientation(false); dirty = true;
			}
			if (dx || dy) {
				uint8_t nx = (uint8_t)(px + dx);
				uint8_t ny = (uint8_t)(py + dy);
				if (!blocked(room, vertical, nx, ny)) { px = nx; py = ny; }
				else rf_beep(100, 3);
				dirty = true;
			}
			if (px == key_x(room) && py == key_y(room)) key = true;
			if (key && px == 10 && py == 1) {
				if (++room >= 5) result = 1;
				else { px = 1; py = 1; key = false; }
				dirty = true;
			}
		}
		if (dirty) {
			render(room < 5 ? room : 4, vertical, px, py, key, result);
			dirty = false;
		}
	}
}
