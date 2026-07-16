#include <stdbool.h>
#include <stdint.h>

#include "rf_swan.h"
#include "gfx.h"

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
	uint8_t intro;

	rf_init(false);
	gfx_show_intro();
	for (intro = 0; intro < 36; ++intro) {
		rf_frame();
		if (rf_input()->pressed) break;
	}
	gfx_init();
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
			gfx_render(camera, kaiju, behavior, disturbance, sun, evidence, zoom, result);
			dirty = false;
		}
	}
}
