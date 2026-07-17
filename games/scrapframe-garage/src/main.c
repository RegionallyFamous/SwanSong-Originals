#include <stdbool.h>
#include <stdint.h>

#include <swan/swan.h>

#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

static scrapframe_state_t state;

void swan_game_boot(void) {
	scrapframe_reset(&state);
	swan_game_audio_init();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_REPAIR) {
		gfx_init();
		swan_core_reset_session();
	}
	swan_core_invalidate();
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	scrapframe_input_t model_input = {0};
	scrapframe_event_t event;

	if (scene == SWAN_SCENE_INTRO) {
		if (swan_game_intro_complete(frame))
			(void)swan_core_request_scene(SWAN_SCENE_REPAIR, 0);
		return;
	}

	model_input.selection_direction = swan_input_dx(frame->input->pressed);
	model_input.confirm = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_CONFIRM);
	scrapframe_step(&state, &model_input, &event);
	if (event.tone_frames)
		swan_game_audio_beep(event.tone_hz, event.tone_frames);
	if (event.dirty) swan_core_invalidate();
	if (event.reset_session)
		(void)swan_core_request_scene(SWAN_SCENE_REPAIR, 0);
	else if (scene == SWAN_SCENE_REPAIR && state.phase == 2)
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		gfx_show_intro();
		return;
	}
	gfx_render(state.job, state.selected, state.score, state.phase,
		state.last_ok);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}
