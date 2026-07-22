#include <stdbool.h>
#include <stdint.h>

#include <swan/swan.h>

#include "swan_assets.h"
#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

#define SFX_END \
	{{SWAN_AUDIO_NOTE_OFF, SWAN_AUDIO_NO_CHANGE, 0}, 1}

static const swan_sfx_step_t SWAN_FAR lane_steps[] = {
	{{46, 2, 5}, 2}, SFX_END
};
static const swan_sfx_step_t SWAN_FAR countdown_steps[] = {
	{{38, 0, 7}, 5}, SFX_END
};
static const swan_sfx_step_t SWAN_FAR go_steps[] = {
	{{45, 0, 9}, 4}, {{50, 0, 9}, 6}, SFX_END
};
static const swan_sfx_step_t SWAN_FAR collision_steps[] = {
	{{10, 2, 10}, 3}, {{7, 2, 8}, 5}, SFX_END
};
static const swan_sfx_step_t SWAN_FAR overtake_steps[] = {
	{{45, 0, 7}, 2}, {{50, 0, 7}, 3}, SFX_END
};
static const swan_sfx_step_t SWAN_FAR checkpoint_steps[] = {
	{{38, 0, 8}, 3}, {{43, 0, 8}, 3}, {{50, 0, 9}, 6}, SFX_END
};
static const swan_sfx_step_t SWAN_FAR tow_steps[] = {
	{{33, 1, 8}, 4}, {{38, 1, 8}, 4}, {{45, 0, 9}, 8}, SFX_END
};
static const swan_sfx_step_t SWAN_FAR finish_steps[] = {
	{{38, 0, 9}, 4}, {{45, 0, 9}, 4}, {{50, 0, 10}, 4},
	{{57, 0, 10}, 12}, SFX_END
};
static const swan_sfx_step_t SWAN_FAR failure_steps[] = {
	{{24, 1, 8}, 6}, {{19, 1, 7}, 6}, {{14, 1, 6}, 12}, SFX_END
};
static const swan_sfx_step_t SWAN_FAR pause_steps[] = {
	{{31, 2, 5}, 3}, SFX_END
};

static const swan_sfx_t SWAN_FAR lane_sfx = {lane_steps, 2, 2};
static const swan_sfx_t SWAN_FAR countdown_sfx = {countdown_steps, 2, 3};
static const swan_sfx_t SWAN_FAR go_sfx = {go_steps, 3, 5};
static const swan_sfx_t SWAN_FAR collision_sfx = {collision_steps, 3, 7};
static const swan_sfx_t SWAN_FAR overtake_sfx = {overtake_steps, 3, 4};
static const swan_sfx_t SWAN_FAR checkpoint_sfx = {checkpoint_steps, 4, 6};
static const swan_sfx_t SWAN_FAR tow_sfx = {tow_steps, 4, 8};
static const swan_sfx_t SWAN_FAR finish_sfx = {finish_steps, 5, 10};
static const swan_sfx_t SWAN_FAR failure_sfx = {failure_steps, 4, 10};
static const swan_sfx_t SWAN_FAR pause_sfx = {pause_steps, 2, 4};
static const swan_audio_sfx_policy_t race_sfx_policy = {
	.preferred_channel = 3,
	.reserved_channel_mask = 0,
	.music_steal_channel_mask = (uint8_t)(1u << 3),
	.music_duck_volume = 12,
	.music_priority = {10, 8, 6, 0},
};

static lap_state_t state;
static bool title_prompt = true;
static uint8_t pause_audio_delay;

#if SWAN_DETERMINISTIC_TRACE
static void mark_trace(lap_sfx_t effect) {
	uint16_t progress = state.distance_total >=
		(uint32_t)LAP_TRACK_UNITS * LAP_TOTAL_LAPS ? 1000u :
		(uint16_t)((state.distance_total * 1000u) /
			((uint32_t)LAP_TRACK_UNITS * LAP_TOTAL_LAPS));
	swan_debug_frame_mark_state(progress, lap_state_hash(&state));
	swan_debug_frame_mark_ending((uint8_t)state.result);
	if (effect != LAP_SFX_NONE)
		swan_debug_frame_mark_audio((uint16_t)(1u << ((uint8_t)effect - 1u)));
}
#else
#define mark_trace(effect) ((void)0)
#endif

static const swan_sfx_t SWAN_FAR *effect_for(lap_sfx_t effect) {
	switch (effect) {
		case LAP_SFX_LANE: return &lane_sfx;
		case LAP_SFX_COUNTDOWN: return &countdown_sfx;
		case LAP_SFX_GO: return &go_sfx;
		case LAP_SFX_COLLISION: return &collision_sfx;
		case LAP_SFX_OVERTAKE: return &overtake_sfx;
		case LAP_SFX_CHECKPOINT: return &checkpoint_sfx;
		case LAP_SFX_TOW: return &tow_sfx;
		case LAP_SFX_FINISH: return &finish_sfx;
		case LAP_SFX_FAILURE: return &failure_sfx;
		case LAP_SFX_PAUSE: return &pause_sfx;
		default: return 0;
	}
}

static void play_effect(lap_sfx_t effect) {
	const swan_sfx_t SWAN_FAR *sfx = effect_for(effect);
	if (sfx) (void)swan_audio_play_sfx(sfx);
}

static void start_music(void) {
	swan_audio_play_music(&swan_asset_race_song_song);
}

void swan_game_boot(void) {
	lap_reset(&state);
	swan_audio_init(swan_asset_race_song_instruments,
		SWAN_ASSET_RACE_SONG_INSTRUMENT_COUNT);
	swan_audio_set_sfx_policy(&race_sfx_policy);
	start_music();
	gfx_reset_title();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_INTRO) {
		gfx_reset_title();
		title_prompt = true;
		start_music();
	} else if (scene == SWAN_SCENE_RACE) {
		lap_reset(&state);
		pause_audio_delay = 0;
		gfx_init();
		swan_core_reset_session();
		start_music();
	}
	swan_core_invalidate();
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	lap_input_t model_input = {0};
	lap_event_t event;
	lap_phase_t previous_phase;

	if (scene == SWAN_SCENE_INTRO) {
		bool prompt = ((frame->boot_tick / 30u) & 1u) == 0;
		if (prompt != title_prompt) {
			title_prompt = prompt;
			swan_core_invalidate();
		}
		if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_START_GAME))
			(void)swan_core_request_scene(SWAN_SCENE_RACE, 0);
		mark_trace(LAP_SFX_NONE);
		return;
	}

	if (scene == SWAN_SCENE_RESULT) {
		if (SWAN_GAME_ACTION_PRESSED(frame->input,
			SWAN_ACTION_ACCELERATE_OR_REPLAY)) {
			(void)swan_core_request_scene(SWAN_SCENE_RACE, 0);
		} else if (SWAN_GAME_ACTION_PRESSED(frame->input,
			SWAN_ACTION_RETURN_TITLE)) {
			(void)swan_core_request_scene(SWAN_SCENE_INTRO, 0);
		}
		mark_trace(LAP_SFX_NONE);
		return;
	}

	model_input.lane_direction =
		(int8_t)SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_RIGHT) -
		(int8_t)SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_LEFT);
	model_input.accelerate = SWAN_GAME_ACTION_HELD(frame->input,
		SWAN_ACTION_ACCELERATE_OR_REPLAY);
	model_input.brake = SWAN_GAME_ACTION_HELD(frame->input, SWAN_ACTION_BRAKE);
	model_input.tow = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_TOW);
	model_input.pause = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_PAUSE);
	previous_phase = state.phase;
	lap_step(&state, &model_input, frame->session_tick, &event);
	mark_trace(event.sfx);

	if (event.sfx == LAP_SFX_PAUSE) {
		if (state.phase == LAP_PHASE_PAUSED) {
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
	if (state.phase == LAP_PHASE_PAUSED && pause_audio_delay &&
		--pause_audio_delay == 0) (void)swan_audio_pause();
	if (event.dirty || (state.phase == LAP_PHASE_RACING &&
		(frame->session_tick & 7u) == 0)) swan_core_invalidate();
	if (previous_phase != LAP_PHASE_RESULT && state.phase == LAP_PHASE_RESULT)
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		gfx_show_intro(title_prompt);
		return;
	}
	gfx_render(&state);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}

#undef SFX_END
#undef mark_trace
