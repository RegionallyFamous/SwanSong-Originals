#include <stdbool.h>
#include <stdint.h>

#include <swan/swan.h>

#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

static radio_state_t state;

void swan_game_boot(void) {
	radio_reset(&state);
	swan_game_audio_init();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_TUNING) {
		gfx_init();
		swan_core_reset_session();
	}
	swan_core_invalidate();
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	radio_input_t model_input = {0};
	radio_event_t event;
	uint16_t directional;

	if (scene == SWAN_SCENE_INTRO) {
		if (swan_game_intro_complete(frame))
			(void)swan_core_request_scene(SWAN_SCENE_TUNING, 0);
		return;
	}

	directional = frame->session_tick % 3u == 0 ?
		frame->input->held : frame->input->pressed;
	model_input.frequency_direction = swan_input_dx(directional);
	model_input.gain_direction = swan_input_dy(directional);
	model_input.lock = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_LOCK_SIGNAL);
	model_input.toggle_gate = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_TOGGLE_GATE);
	model_input.replay = model_input.lock;
	radio_step(&state, &model_input, &event);
	if (event.tone_frames)
		swan_game_audio_beep(event.tone_hz, event.tone_frames);
	if (event.dirty || (state.result == RADIO_RESULT_PLAYING &&
		(frame->session_tick & 15u) == 0)) swan_core_invalidate();
	if (event.reset_session)
		(void)swan_core_request_scene(SWAN_SCENE_TUNING, 0);
	else if (scene == SWAN_SCENE_TUNING && state.result != RADIO_RESULT_PLAYING)
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		gfx_show_intro();
		return;
	}
	gfx_render(state.frequency, state.gain, state.time, state.clue,
		(uint8_t)state.result, state.gate);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}
