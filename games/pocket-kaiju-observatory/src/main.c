#include <stdbool.h>
#include <stdint.h>

#include <swan/swan.h>

#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

static kaiju_state_t state;

void swan_game_boot(void) {
	kaiju_reset(&state);
	swan_game_audio_init();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_OBSERVATION) {
		gfx_init();
		swan_core_reset_session();
	}
	swan_core_invalidate();
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	kaiju_input_t model_input = {0};
	kaiju_event_t event;

	if (scene == SWAN_SCENE_INTRO) {
		if (swan_game_intro_complete(frame))
			(void)swan_core_request_scene(SWAN_SCENE_OBSERVATION, 0);
		return;
	}

	model_input.direction = swan_game_primary_axis(frame->input->pressed);
	model_input.photograph = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_PHOTOGRAPH);
	model_input.hide = SWAN_GAME_ACTION_HELD(frame->input, SWAN_ACTION_HIDE);
	model_input.toggle_zoom = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_TOGGLE_ZOOM);
	model_input.replay = model_input.photograph;
	kaiju_step(&state, &model_input, frame->session_tick, &event);
	if (event.tone_frames)
		swan_game_audio_beep(event.tone_hz, event.tone_frames);
	if (event.dirty || (frame->session_tick & 15u) == 0) swan_core_invalidate();
	if (event.reset_session)
		(void)swan_core_request_scene(SWAN_SCENE_OBSERVATION, 0);
	else if (scene == SWAN_SCENE_OBSERVATION && state.result != KAIJU_RESULT_PLAYING)
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		gfx_show_intro();
		return;
	}
	gfx_render(state.camera, state.kaiju, state.behavior, state.disturbance,
		state.sun, state.evidence, state.zoom, (uint8_t)state.result);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}
