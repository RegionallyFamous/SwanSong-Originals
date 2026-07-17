#include <stdbool.h>
#include <stdint.h>

#include <swan/swan.h>

#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

static courier_state_t state;

void swan_game_boot(void) {
	courier_reset(&state);
	swan_game_audio_init();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_DELIVERY) {
		orbital_gfx_init();
		swan_core_reset_session();
	}
	swan_core_invalidate();
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	courier_input_t model_input = {0};
	courier_event_t event;

	if (scene == SWAN_SCENE_INTRO) {
		if (swan_game_intro_complete(frame))
			(void)swan_core_request_scene(SWAN_SCENE_DELIVERY, 0);
		return;
	}
	if (scene == SWAN_SCENE_RESULT) {
		if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_REPLAY)) {
			courier_reset(&state);
			(void)swan_core_request_scene(SWAN_SCENE_DELIVERY, 0);
		}
		return;
	}

	model_input.dx = swan_input_dx(frame->input->pressed);
	model_input.dy = swan_input_dy(frame->input->pressed);
	courier_step(&state, &model_input, &event);
	if (event.tone_frames)
		swan_game_audio_beep(event.tone_hz, event.tone_frames);
	if (event.dirty) swan_core_invalidate();
	if (state.result != COURIER_RESULT_PLAYING)
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		orbital_gfx_show_intro();
		return;
	}
	orbital_gfx_render(state.x, state.y, state.parcel, state.fuel,
		state.steps, (uint8_t)state.result);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}
