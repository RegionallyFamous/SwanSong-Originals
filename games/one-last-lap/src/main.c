#include <stdbool.h>
#include <stdint.h>

#include <swan/swan.h>

#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

static lap_state_t state;

void swan_game_boot(void) {
	lap_reset(&state);
	swan_game_audio_init();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_RACE) {
		gfx_init();
		swan_core_reset_session();
	}
	swan_core_invalidate();
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	lap_input_t model_input = {0};
	lap_event_t event;

	if (scene == SWAN_SCENE_INTRO) {
		if (swan_game_intro_complete(frame))
			(void)swan_core_request_scene(SWAN_SCENE_RACE, 0);
		return;
	}
	if (scene == SWAN_SCENE_RESULT) {
		if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_ACCELERATE_OR_REPLAY)) {
			lap_reset(&state);
			(void)swan_core_request_scene(SWAN_SCENE_RACE, 0);
		}
		if ((frame->session_tick & 15u) == 0) swan_core_invalidate();
		return;
	}

	model_input.lane_direction = swan_game_primary_axis(frame->input->pressed);
	model_input.accelerate = SWAN_GAME_ACTION_HELD(frame->input, SWAN_ACTION_ACCELERATE_OR_REPLAY);
	model_input.brake = SWAN_GAME_ACTION_HELD(frame->input, SWAN_ACTION_BRAKE);
	model_input.tow = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_TOW);
	lap_step(&state, &model_input, frame->session_tick, &event);
	if (event.tone_frames)
		swan_game_audio_beep(event.tone_hz, event.tone_frames);
	if (event.dirty || (frame->session_tick & 15u) == 0) swan_core_invalidate();
	if (state.result != LAP_RESULT_PLAYING)
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		gfx_show_intro();
		return;
	}
	gfx_render(state.lap > 3 ? 3 : state.lap, state.progress, state.speed,
		state.battery, state.lane, state.helped, (uint8_t)state.result);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}
