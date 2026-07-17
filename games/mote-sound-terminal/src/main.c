#include <stdbool.h>
#include <stdint.h>

#include <swan/swan.h>

#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

static mote_state_t state;

void swan_game_boot(void) {
	mote_reset(&state);
	swan_game_audio_init();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_TERMINAL) {
		gfx_init();
		swan_core_reset_session();
	}
	swan_core_invalidate();
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	mote_input_t model_input = {0};
	mote_event_t event;

	if (scene == SWAN_SCENE_INTRO) {
		if (swan_game_intro_complete(frame))
			(void)swan_core_request_scene(SWAN_SCENE_TERMINAL, 0);
		return;
	}

	model_input.track_direction = swan_input_dx(frame->input->pressed);
	model_input.tempo_direction = swan_input_dy(frame->input->pressed);
	model_input.toggle_play = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_TOGGLE_PLAY);
	model_input.toggle_scope = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_TOGGLE_SCOPE);
	model_input.reset = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_RESET);
	mote_step(&state, &model_input, &event);
	if (event.sound_off) swan_game_audio_off();
	if (event.reset_session) swan_core_reset_session();
	if (event.tone_hz) swan_game_audio_tone(event.tone_hz, event.tone_volume);
	if (event.dirty) swan_core_invalidate();
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		gfx_show_intro();
		return;
	}
	gfx_render(state.track, state.playing, state.tempo, state.scope, state.step);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}
