#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "rf_swan.h"
#include "native_art.h"

static const char __far title[] = "SCRAPFRAME GARAGE";
static const char __far subtitle[] = "One shift. Three repairs.";
static const char __far help[] = "Left/right part  A install";
static const char __far fmt_job[] = "JOB %u/3   CREDITS %u\n\n";
static const char __far fmt_select[] = "PART: %s\n\n";
static const char __far fmt_score[] = "SHIFT COMPLETE\n\n%u/3 repairs passed.\n";
static const char __far part_joint[] = "SERVO JOINT";
static const char __far part_memory[] = "MEMORY TILE";
static const char __far part_cooler[] = "COOLANT FAN";
static const char __far passed[] = "TEST PASSED! A: next job";
static const char __far failed[] = "SIDE EFFECT! A: next job";

static void print_robot(uint8_t job) {
	if (job == 0) {
		printf("CASE MOP-4\nWHEEL WOBBLE\nCHECK DRIVE SIDE\n");
	} else if (job == 1) {
		printf("CASE LUX-9\nMEMORY RESETS\nCHECK STORAGE\n");
	} else {
		printf("CASE KET-2\nCASING TOO HOT\nCHECK AIRFLOW\n");
	}
}

static const char __far *part_name(uint8_t part) {
	if (part == 0) return part_joint;
	if (part == 1) return part_memory;
	return part_cooler;
}

static void render(uint8_t job, uint8_t selected, uint8_t score,
	uint8_t phase, bool last_ok) {
	rf_clear();
	rf_header(title, subtitle);
	if (phase == 2) {
		printf(fmt_score, score);
		printf("A: work another shift\n");
		rf_footer(help);
		return;
	}
	printf(fmt_job, job + 1, score * 12);
	print_robot(job);
	putchar('\n');
	printf("PARTS JNT MEM FAN\n");
	printf(fmt_select, part_name(selected));
	if (phase == 1) printf(last_ok ? passed : failed);
	else printf("Match symptom to the part.\n");
	putchar('\n');
	rf_footer(help);
}

void main(void) {
	static const uint8_t correct[3] = {0, 1, 2};
	uint8_t job = 0;
	uint8_t selected = 0;
	uint8_t score = 0;
	uint8_t phase = 0;
	bool last_ok = false;
	bool dirty = true;

	rf_init(false);
	RF_LOAD_NATIVE_ART();
	while (1) {
		const rf_input_t *input;
		int8_t dx;
		rf_frame();
		input = rf_input();
		dx = rf_dx(input->pressed);

		if (phase == 0) {
			if (dx) {
				selected = (uint8_t)((selected + (dx > 0 ? 1 : 2)) % 3);
				dirty = true;
			}
			if (input->pressed & WS_KEY_A) {
				last_ok = selected == correct[job];
				if (last_ok) { ++score; rf_beep(660, 10); }
				else rf_beep(130, 10);
				phase = 1;
				dirty = true;
			}
		} else if (phase == 1 && (input->pressed & WS_KEY_A)) {
			if (++job >= 3) phase = 2;
			else { phase = 0; selected = 0; }
			dirty = true;
		} else if (phase == 2 && (input->pressed & WS_KEY_A)) {
			job = 0; selected = 0; score = 0; phase = 0;
			dirty = true;
		}

		if (dirty) {
			render(job, selected, score, phase, last_ok);
			dirty = false;
		}
	}
}
