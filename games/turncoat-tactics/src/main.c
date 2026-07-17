#include <stdbool.h>
#include <stdint.h>

#include <swan/swan.h>

#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

static turncoat_state_t state;

void swan_game_boot(void) {
	turncoat_reset(&state);
	swan_game_audio_init();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_BATTLE) {
		gfx_init();
		swan_core_reset_session();
	}
	swan_core_invalidate();
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	turncoat_input_t model_input = {0};
	turncoat_event_t event;

	if (scene == SWAN_SCENE_INTRO) {
		if (swan_game_intro_complete(frame))
			(void)swan_core_request_scene(SWAN_SCENE_BATTLE, 0);
		return;
	}

	model_input.dx = swan_input_dx(frame->input->pressed);
	model_input.dy = swan_input_dy(frame->input->pressed);
	model_input.select_or_act = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_SELECT_OR_ACT);
	model_input.recruit = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_RECRUIT);
	model_input.end_turn = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_END_TURN);
	model_input.replay = model_input.select_or_act;
	turncoat_step(&state, &model_input, &event);
	if (event.tone_frames)
		swan_game_audio_beep(event.tone_hz, event.tone_frames);
	if (event.dirty) swan_core_invalidate();
	if (event.reset_session)
		(void)swan_core_request_scene(SWAN_SCENE_BATTLE, 0);
	else if (scene == SWAN_SCENE_BATTLE && state.result != TURNCOAT_RESULT_PLAYING)
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		gfx_show_intro();
		return;
	}
	gfx_render(&state);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}
