#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "rf_swan.h"
#include "native_art.h"

static const char __far title[] = "POCKET KAIJU OBSERVATORY";
static const char __far subtitle[] = "Observe. Do not disturb.";
static const char __far help[] = "Move Ashot Bhide Start zoom";
static const char __far fmt_status[] = "SUN %u DIST %u DATA %u/3\n";
static const char __far fmt_behavior[] = "BEHAVIOR %s\nZOOM %ux\n";
static const char __far sleep_text[] = "SLEEP";
static const char __far feed_text[] = "FEED";
static const char __far migrate_text[] = "MIGRATE";

static const char __far *behavior_name(uint8_t behavior) {
	if (behavior == 0) return sleep_text;
	if (behavior == 1) return feed_text;
	return migrate_text;
}

static uint8_t bit_count(uint8_t bits) {
	uint8_t count = 0;
	while (bits) { count += bits & 1; bits >>= 1; }
	return count;
}

static void render(uint8_t camera, uint8_t kaiju, uint8_t behavior,
	uint8_t disturbance, uint16_t sun, uint8_t evidence, bool zoom,
	uint8_t result) {
	uint8_t x;
	rf_clear();
	rf_header(title, subtitle);
	printf(fmt_status, sun / 75, disturbance, bit_count(evidence));
	printf(fmt_behavior, behavior_name(behavior), zoom ? 2 : 1);
	rf_playfield_begin();
	for (x = 0; x < 21; ++x) {
		char c = '.';
		if (x == kaiju) c = 'K';
		if (x == camera) c = 'C';
		putchar(c);
	}
	putchar('\n');
	rf_playfield_end();
	printf("Frame each behavior\n");
	printf("at safe distance.\n\n");
	if (result == 1) printf("DOSSIER COMPLETE! A again\n");
	else if (result == 2) printf("EXPEDITION ENDED. A retry\n");
	else printf("B lowers disturbance.\n");
	rf_footer(help);
}

void main(void) {
	uint8_t camera = 2;
	uint8_t kaiju = 15;
	uint8_t behavior = 0;
	uint8_t disturbance = 0;
	uint8_t evidence = 0;
	uint8_t result = 0;
	uint16_t sun = 1800;
	bool zoom = false;
	bool dirty = true;

	rf_init(false);
	RF_LOAD_NATIVE_ART();
	while (1) {
		const rf_input_t *input;
		rf_frame();
		input = rf_input();

		if (result && (input->pressed & WS_KEY_A)) {
			camera = 2; kaiju = 15; behavior = 0; disturbance = 0;
			evidence = 0; result = 0; sun = 1800; zoom = false; dirty = true;
		}
		if (!result) {
			int8_t dx = rf_dx(input->pressed);
			if (dx) { camera = rf_clamp_u8((int16_t)camera + dx, 0, 20); dirty = true; }
			if (input->pressed & WS_KEY_START) { zoom = !zoom; dirty = true; }
			if ((input->held & WS_KEY_B) && disturbance && (rf_frame_count() & 3) == 0) {
				--disturbance; dirty = true;
			}
			if (input->pressed & WS_KEY_A) {
				uint8_t distance = camera > kaiju ? camera - kaiju : kaiju - camera;
				uint8_t low = zoom ? 6 : 3;
				uint8_t high = zoom ? 10 : 7;
				disturbance = rf_clamp_u8((int16_t)disturbance + 24, 0, 100);
				if (distance >= low && distance <= high) {
					evidence |= (uint8_t)(1 << behavior);
					rf_beep(700, 8);
				} else rf_beep(140, 6);
				dirty = true;
			}
			if ((rf_frame_count() % 150) == 0) {
				behavior = (uint8_t)((behavior + 1) % 3);
				dirty = true;
			}
			if (behavior == 2 && (rf_frame_count() % 20) == 0) {
				kaiju = (uint8_t)(8 + rf_random() % 12);
				dirty = true;
			}
			if (sun) --sun;
			if ((evidence & 7) == 7) result = 1;
			else if (disturbance >= 100 || sun == 0) result = 2;
		}
		if (dirty || (rf_frame_count() & 15) == 0) {
			render(camera, kaiju, behavior, disturbance, sun, evidence, zoom, result);
			dirty = false;
		}
	}
}
