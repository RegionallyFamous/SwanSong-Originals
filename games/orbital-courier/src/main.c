#include <stdbool.h>
#include <stdint.h>

#include <swan/swan.h>

#include "swan_assets.h"
#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

static const swan_audio_sfx_policy_t courier_sfx_policy = {
	.preferred_channel = 3,
	.reserved_channel_mask = 0,
	.music_steal_channel_mask = (uint8_t)(1u << 3),
	.music_duck_volume = 11,
	.music_priority = {10, 8, 6, 0},
};

static courier_state_t state;
static bool title_prompt = true;
static uint8_t pause_audio_delay;

static bool action_pressed(const swan_frame_t *frame, uint8_t action) {
	return (frame->input->actions_pressed & (uint16_t)(1u << action)) != 0;
}

static bool action_move(const swan_frame_t *frame, uint8_t action) {
	uint16_t bits = (uint16_t)(frame->input->actions_pressed |
		frame->input->actions_repeated);
	return (bits & (uint16_t)(1u << action)) != 0;
}

#if SWAN_DETERMINISTIC_TRACE
static void mark_trace(courier_sfx_t effect) {
	swan_debug_frame_mark_state(courier_progress(&state),
		courier_state_hash(&state));
	swan_debug_frame_mark_ending((uint8_t)state.result);
	if (effect != COURIER_SFX_NONE)
		swan_debug_frame_mark_audio((uint16_t)(1u << ((uint8_t)effect - 1u)));
}
#else
#define mark_trace(effect) ((void)0)
#endif

static const swan_sfx_t SWAN_FAR *effect_for(courier_sfx_t effect) {
	switch (effect) {
		case COURIER_SFX_MOVE: return &swan_asset_move_sfx;
		case COURIER_SFX_BLOCKED: return &swan_asset_blocked_sfx;
		case COURIER_SFX_PICKUP: return &swan_asset_pickup_sfx;
		case COURIER_SFX_CHARGE: return &swan_asset_charge_sfx;
		case COURIER_SFX_RELAY: return &swan_asset_relay_sfx;
		case COURIER_SFX_COLLISION: return &swan_asset_collision_sfx;
		case COURIER_SFX_PAUSE: return &swan_asset_pause_sfx;
		case COURIER_SFX_RETRY: return &swan_asset_retry_sfx;
		case COURIER_SFX_DELIVERED: return &swan_asset_delivered_sfx;
		case COURIER_SFX_FAILURE: return &swan_asset_failure_sfx;
		default: return 0;
	}
}

static void play_effect(courier_sfx_t effect) {
	const swan_sfx_t SWAN_FAR *sfx = effect_for(effect);
	if (sfx) (void)swan_audio_play_sfx(sfx);
}

static void start_title_music(void) {
	swan_audio_init(swan_asset_title_music_instruments,
		SWAN_ASSET_TITLE_MUSIC_INSTRUMENT_COUNT);
	swan_audio_set_sfx_policy(&courier_sfx_policy);
	swan_audio_play_music(&swan_asset_title_music_song);
}

static void start_delivery_music(void) {
	swan_audio_init(swan_asset_delivery_music_instruments,
		SWAN_ASSET_DELIVERY_MUSIC_INSTRUMENT_COUNT);
	swan_audio_set_sfx_policy(&courier_sfx_policy);
	swan_audio_play_music(&swan_asset_delivery_music_song);
}

void swan_game_boot(void) {
	courier_reset(&state);
	orbital_gfx_reset_title();
	start_title_music();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_INTRO) {
		title_prompt = true;
		orbital_gfx_reset_title();
		start_title_music();
	} else if (scene == SWAN_SCENE_DELIVERY) {
		courier_reset(&state);
		pause_audio_delay = 0;
		orbital_gfx_init();
		swan_core_reset_session();
		start_delivery_music();
	}
	swan_core_invalidate();
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	courier_input_t model_input = {0};
	courier_event_t event;
	courier_phase_t previous_phase;

	if (scene == SWAN_SCENE_INTRO) {
		bool prompt = ((frame->boot_tick / 30u) & 1u) == 0;
		if (prompt != title_prompt) {
			title_prompt = prompt;
			swan_core_invalidate();
		}
		if (action_pressed(frame, SWAN_ACTION_START_OR_PAUSE))
			(void)swan_core_request_scene(SWAN_SCENE_DELIVERY, 0);
		mark_trace(COURIER_SFX_NONE);
		return;
	}

	if (scene == SWAN_SCENE_RESULT) {
		if (action_pressed(frame, SWAN_ACTION_REPLAY)) {
			play_effect(COURIER_SFX_RETRY);
			mark_trace(COURIER_SFX_RETRY);
			(void)swan_core_request_scene(SWAN_SCENE_DELIVERY, 0);
		} else if (action_pressed(frame, SWAN_ACTION_START_OR_PAUSE)) {
			(void)swan_core_request_scene(SWAN_SCENE_INTRO, 0);
			mark_trace(COURIER_SFX_NONE);
		} else {
			mark_trace(COURIER_SFX_NONE);
		}
		return;
	}

	model_input.dx = (int8_t)action_move(frame, SWAN_ACTION_RIGHT) -
		(int8_t)action_move(frame, SWAN_ACTION_LEFT);
	model_input.dy = (int8_t)action_move(frame, SWAN_ACTION_DOWN) -
		(int8_t)action_move(frame, SWAN_ACTION_UP);
	model_input.pause = action_pressed(frame, SWAN_ACTION_START_OR_PAUSE);
	model_input.retry = action_pressed(frame, SWAN_ACTION_RETRY);
	previous_phase = state.phase;
	courier_step(&state, &model_input, frame->session_tick, &event);
	mark_trace(event.sfx);

	if (event.sfx == COURIER_SFX_PAUSE) {
		if (state.phase == COURIER_PHASE_PAUSED) {
			play_effect(event.sfx);
			pause_audio_delay = 3;
		} else {
			pause_audio_delay = 0;
			(void)swan_audio_resume();
			play_effect(event.sfx);
		}
	} else {
		play_effect(event.sfx);
	}
	if (state.phase == COURIER_PHASE_PAUSED && pause_audio_delay &&
		--pause_audio_delay == 0) (void)swan_audio_pause();
	if (event.reset) {
		pause_audio_delay = 0;
		swan_core_reset_session();
		start_delivery_music();
		play_effect(COURIER_SFX_RETRY);
	}
	if (event.dirty || (state.phase == COURIER_PHASE_ACTIVE &&
		(frame->session_tick & 7u) == 0)) swan_core_invalidate();
	if (previous_phase != COURIER_PHASE_RESULT &&
		state.phase == COURIER_PHASE_RESULT)
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		orbital_gfx_show_intro(title_prompt);
		return;
	}
	orbital_gfx_render(&state);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}

#undef SFX_END
#undef mark_trace
