#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "rf_swan.h"
#include "native_art.h"

static const char __far title[] = "ONE LAST LAP";
static const char __far subtitle[] = "Old couriers never quit";
static const char __far help[] = "A gas B brake D-pad lane";
static const char __far fmt_status[] = "LAP %u/3 SPD %u BAT %u\n";

static void render(uint8_t lap, uint8_t progress, uint8_t speed,
	uint8_t battery, uint8_t lane, bool helped, uint8_t result) {
	uint8_t row;
	rf_clear();
	rf_header(title, subtitle);
	printf(fmt_status, lap, speed, battery);
	printf("TRACK ");
	rf_print_bar(progress, 100, 10);
	putchar('\n');
	printf("RIV %u/%u/%u\n", (progress + 13) % 100, (progress + 31) % 100,
		(progress + 57) % 100);
	printf("\n");
	rf_playfield_begin();
	for (row = 0; row < 3; ++row) {
		printf(row == lane ? "|======[@]======|\n" : "|======[.]======|\n");
	}
	rf_playfield_end();
	printf("\nRIVAL %s\n", helped ? "TOWED" : "WAIT @48");
	if (result == 1 && helped) printf("YOU FINISHED TOGETHER.\nA: race again\n");
	else if (result == 1) printf("YOU WON, BUT DROVE ALONE.\nA: race again\n");
	else if (result == 2) printf("BATTERY COLD. A: retry\n");
	else printf("START near rival to tow.\n");
	rf_footer(help);
}

void main(void) {
	uint8_t lap = 1;
	uint8_t progress = 0;
	uint8_t speed = 0;
	uint8_t battery = 70;
	uint8_t lane = 1;
	uint8_t result = 0;
	bool helped = false;
	bool crash_zone = false;
	bool dirty = true;

	rf_init(false);
	RF_LOAD_NATIVE_ART();
	while (1) {
		const rf_input_t *input;
		rf_frame();
		input = rf_input();

		if (result && (input->pressed & WS_KEY_A)) {
			lap = 1; progress = 0; speed = 0; battery = 70; lane = 1;
			result = 0; helped = false; crash_zone = false; dirty = true;
		}
		if (!result) {
			int8_t dx = rf_dx(input->pressed);
			if (dx) { lane = rf_clamp_u8((int16_t)lane + dx, 0, 2); dirty = true; }
			if ((input->held & WS_KEY_A) && (rf_frame_count() % 10) == 0 && speed < 6 && battery) {
				++speed; dirty = true;
			}
			if ((input->held & WS_KEY_B) && (rf_frame_count() % 6) == 0 && speed) {
				--speed; dirty = true;
			}
			if ((input->pressed & WS_KEY_START) && !helped && lap == 2 && progress >= 40 && progress <= 56) {
				helped = true; speed = 0; battery = battery > 10 ? battery - 10 : 0;
				rf_beep(560, 12); dirty = true;
			}
			if ((rf_frame_count() & 7) == 0 && speed) {
				uint16_t next = (uint16_t)progress + speed;
				if (next >= 100) {
					progress = (uint8_t)(next - 100);
					if (++lap > 3) result = 1;
				} else progress = (uint8_t)next;
				if (battery) --battery;
				dirty = true;
			}
			if (!crash_zone && ((progress >= 24 && progress <= 30 && lane == 1) ||
				(progress >= 68 && progress <= 74 && lane == 2))) {
				speed = 1; battery = battery > 6 ? battery - 6 : 0; crash_zone = true;
				rf_beep(100, 8); dirty = true;
			}
			if (!((progress >= 20 && progress <= 34) || (progress >= 64 && progress <= 78))) crash_zone = false;
			if (battery == 0) { speed = 0; result = 2; dirty = true; }
		}
		if (dirty || (rf_frame_count() & 15) == 0) {
			render(lap > 3 ? 3 : lap, progress, speed, battery, lane, helped, result);
			dirty = false;
		}
	}
}
