#include <stdbool.h>
#include <stdint.h>

#include <swan/swan.h>

#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

static harpoon_state_t state;

void swan_game_boot(void) {
	harpoon_reset(&state);
	swan_game_audio_init();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_HUNT) {
		gfx_init();
		swan_core_reset_session();
	}
	swan_core_invalidate();
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	harpoon_input_t model_input = {0};
	harpoon_event_t event;

	if (scene == SWAN_SCENE_INTRO) {
		if (swan_game_intro_complete(frame))
			(void)swan_core_request_scene(SWAN_SCENE_HUNT, 0);
		return;
	}

	model_input.direction = swan_game_primary_axis(frame->input->pressed);
	model_input.charge_held = SWAN_GAME_ACTION_HELD(frame->input, SWAN_ACTION_CHARGE_FIRE);
	model_input.lure_held = SWAN_GAME_ACTION_HELD(frame->input, SWAN_ACTION_LURE);
	model_input.fire_released = SWAN_GAME_ACTION_RELEASED(frame->input, SWAN_ACTION_CHARGE_FIRE);
	model_input.replay = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_CHARGE_FIRE);
	harpoon_step(&state, &model_input, frame->session_tick, &event);
	if (event.tone_frames)
		swan_game_audio_beep(event.tone_hz, event.tone_frames);
	if (event.dirty || (frame->session_tick & 7u) == 0) swan_core_invalidate();
	if (event.reset_session)
		(void)swan_core_request_scene(SWAN_SCENE_HUNT, 0);
	else if (scene == SWAN_SCENE_HUNT && state.result != HARPOON_RESULT_PLAYING)
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		gfx_show_intro();
		return;
	}
	gfx_render(state.skiff, state.creature, state.oxygen, state.tags,
		state.boss_hp, state.charge, (uint8_t)state.result);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}
