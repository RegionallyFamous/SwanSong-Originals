#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "rf_swan.h"

static const char __far title[] = "RADIO GHOST";
static const char __far subtitle[] = "Night shift ends at dawn";
static const char __far help[] = "D-pad tune/gain  A lock";
static const char __far fmt_dial[] = "DIAL %u.%u FM   GAIN %u\n";
static const char __far fmt_time[] = "DAWN IN %u   CLUES %u/3\n";
static const char __far fmt_signal[] = "SIGNAL: %s\n\n";
static const char __far static_text[] = "static";
static const char __far near_text[] = "voice beneath the noise";

#define FRAME_RATE 75
#define NIGHT_FRAMES 4500

static uint16_t target_for(uint8_t clue) {
	if (clue == 0) return 934;
	if (clue == 1) return 995;
	return 1042;
}

static void render(uint16_t frequency, uint8_t gain, uint16_t time,
	uint8_t clue, uint8_t result, bool gate) {
	uint16_t target = target_for(clue < 3 ? clue : 2);
	uint16_t distance = frequency > target ? frequency - target : target - frequency;
	rf_clear();
	rf_header(title, subtitle);
	printf(fmt_dial, frequency / 10, frequency % 10, gain);
	printf(fmt_time, time / FRAME_RATE, clue);
	rf_print_bar((uint8_t)((frequency - 880) / 10), 20, 20);
	putchar('\n');
	printf(fmt_signal, distance <= (gate ? 8 : 3) ? near_text : static_text);
	if (clue > 0) printf("CLUE 1: The caller knows me.\n");
	if (clue > 1) printf("CLUE 2: The clock runs back.\n");
	if (clue > 2) printf("CLUE 3: Do not say hello.\n");
	if (result == 1) printf("LINE OPEN. You found them.\nA: another night\n");
	else if (result == 2) printf("DAWN. The signal is gone.\nA: try again\n");
	else printf("B toggles wide noise gate.\n");
	rf_footer(help);
}

void main(void) {
	uint16_t frequency = 880;
	uint16_t time = NIGHT_FRAMES;
	uint8_t gain = 5;
	uint8_t clue = 0;
	uint8_t result = 0;
	bool gate = false;
	bool dirty = true;

	rf_init(false);
	while (1) {
		const rf_input_t *input;
		int8_t dx;
		int8_t dy;
		rf_frame();
		input = rf_input();
		dx = rf_dx((rf_frame_count() % 3) == 0 ? input->held : input->pressed);
		dy = rf_dy((rf_frame_count() % 3) == 0 ? input->held : input->pressed);

		if (result && (input->pressed & WS_KEY_A)) {
			frequency = 880; time = NIGHT_FRAMES; gain = 5; clue = 0; result = 0; gate = false;
			dirty = true;
		}
		if (!result) {
			if (dx) {
				frequency = (uint16_t)rf_clamp_u8((int16_t)((frequency - 880) / 2) + dx, 0, 100) * 2 + 880;
				dirty = true;
			}
			if (dy) {
				gain = rf_clamp_u8((int16_t)gain - dy, 0, 9);
				dirty = true;
			}
			if (input->pressed & WS_KEY_B) { gate = !gate; dirty = true; }
			if (input->pressed & WS_KEY_A) {
				uint16_t target = target_for(clue);
				uint16_t distance = frequency > target ? frequency - target : target - frequency;
				if (distance <= 3 && gain >= 3) {
					++clue;
					rf_beep((uint16_t)(440 + clue * 80), 12);
					if (clue == 3) result = 1;
				} else {
					time = time > 300 ? time - 300 : 0;
					rf_beep(110, 8);
				}
				dirty = true;
			}
			if (time) --time;
			else result = 2;
		}
		if (dirty || (rf_frame_count() & 7) == 0) {
			render(frequency, gain, time, clue, result, gate);
			dirty = false;
		}
	}
}
