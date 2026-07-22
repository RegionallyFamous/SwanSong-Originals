#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <swan/swan.h>

#include "swan_assets.h"
#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

#define HARPOON_SAVE_SCHEMA 1u
#define HARPOON_ATTRACT_FRAMES 755u

typedef struct {
	uint16_t best_score;
	uint8_t best_rank;
	uint8_t tutorial_complete;
} harpoon_record_t;

_Static_assert(sizeof(harpoon_record_t) == 4u,
	"Harpoon Moon save record must remain byte-stable");

static const swan_audio_sfx_policy_t harpoon_sfx_policy = {
	.preferred_channel = 3,
	.reserved_channel_mask = 0,
	.music_steal_channel_mask = (uint8_t)(1u << 3),
	.music_duck_volume = 11,
	.music_priority = {10, 8, 6, 0},
};

static harpoon_state_t state;
static bool title_prompt = true;
static bool attract_mode;
static bool training_mode = true;
static uint8_t title_page;
static uint8_t pause_audio_delay;
static uint16_t title_idle_frames;
static swan_ws_eeprom_context_t eeprom_context;
static swan_storage_t cartridge_storage;
static harpoon_record_t records;
static bool storage_ready;

#if SWAN_DETERMINISTIC_TRACE
static void mark_trace(harpoon_sfx_t effect) {
	swan_debug_frame_mark_state(harpoon_progress(&state),
		harpoon_state_hash(&state));
	swan_debug_frame_mark_ending((uint8_t)state.result);
	if (effect != HARPOON_SFX_NONE)
		swan_debug_frame_mark_audio(
			(uint16_t)(1u << ((uint8_t)effect - 1u)));
}
#else
#define mark_trace(effect) ((void)0)
#endif

static const swan_sfx_t SWAN_FAR *effect_for(harpoon_sfx_t effect) {
	switch (effect) {
		case HARPOON_SFX_MOVE: return &swan_asset_move_sfx_sfx;
		case HARPOON_SFX_LURE: return &swan_asset_lure_sfx_sfx;
		case HARPOON_SFX_CHARGE: return &swan_asset_charge_sfx_sfx;
		case HARPOON_SFX_READY: return &swan_asset_ready_sfx_sfx;
		case HARPOON_SFX_SHOT: return &swan_asset_shot_sfx_sfx;
		case HARPOON_SFX_HIT: return &swan_asset_hit_sfx_sfx;
		case HARPOON_SFX_TAG: return &swan_asset_tag_sfx_sfx;
		case HARPOON_SFX_BOSS: return &swan_asset_boss_sfx_sfx;
		case HARPOON_SFX_MISS: return &swan_asset_miss_sfx_sfx;
		case HARPOON_SFX_RECOVER: return &swan_asset_recover_sfx_sfx;
		case HARPOON_SFX_PAUSE: return &swan_asset_pause_sfx_sfx;
		case HARPOON_SFX_RETRY: return &swan_asset_retry_sfx_sfx;
		case HARPOON_SFX_WIN: return &swan_asset_win_sfx_sfx;
		case HARPOON_SFX_FAILURE: return &swan_asset_failure_sfx_sfx;
		default: return 0;
	}
}

static void play_effect(harpoon_sfx_t effect) {
	const swan_sfx_t SWAN_FAR *sfx = effect_for(effect);
	if (sfx) (void)swan_audio_play_sfx(sfx);
}

static void start_title_music(void) {
	swan_audio_init(swan_asset_title_music_instruments,
		SWAN_ASSET_TITLE_MUSIC_INSTRUMENT_COUNT);
	swan_audio_set_sfx_policy(&harpoon_sfx_policy);
	swan_audio_play_music(&swan_asset_title_music_song);
}

static void start_hunt_music(void) {
	swan_audio_init(swan_asset_hunt_music_instruments,
		SWAN_ASSET_HUNT_MUSIC_INSTRUMENT_COUNT);
	swan_audio_set_sfx_policy(&harpoon_sfx_policy);
	swan_audio_play_music(&swan_asset_hunt_music_song);
}

static void start_boss_music(void) {
	swan_audio_init(swan_asset_boss_music_instruments,
		SWAN_ASSET_BOSS_MUSIC_INSTRUMENT_COUNT);
	swan_audio_set_sfx_policy(&harpoon_sfx_policy);
	swan_audio_play_music(&swan_asset_boss_music_song);
}

static void start_result_music(void) {
	swan_audio_init(swan_asset_result_music_instruments,
		SWAN_ASSET_RESULT_MUSIC_INSTRUMENT_COUNT);
	swan_audio_set_sfx_policy(&harpoon_sfx_policy);
	swan_audio_play_music(&swan_asset_result_music_song);
}

static void load_records(void) {
	swan_save_info_t info;
	swan_save_status_t status;
	memset(&records, 0, sizeof(records));
	storage_ready = swan_ws_eeprom_storage(&eeprom_context,
		&cartridge_storage, 128u);
	if (!storage_ready) return;
	status = swan_save_load(&cartridge_storage, HARPOON_SAVE_SCHEMA,
		&records, sizeof(records), &info);
	if (status != SWAN_SAVE_OK || records.best_rank > 3u ||
		records.tutorial_complete > 1u)
		memset(&records, 0, sizeof(records));
}

static void store_records(bool tutorial_complete, bool completed_hunt) {
	harpoon_record_t candidate = records;
	swan_save_info_t info;
	bool changed = false;
	if (tutorial_complete && candidate.tutorial_complete == 0u) {
		candidate.tutorial_complete = 1u;
		changed = true;
	}
	if (completed_hunt && state.result == HARPOON_RESULT_WIN) {
		if (state.score > candidate.best_score) {
			candidate.best_score = state.score;
			changed = true;
		}
		if (state.rank > candidate.best_rank) {
			candidate.best_rank = state.rank;
			changed = true;
		}
	}
	if (changed && storage_ready &&
		swan_save_store(&cartridge_storage, HARPOON_SAVE_SCHEMA,
			&candidate, sizeof(candidate), &info) == SWAN_SAVE_OK)
		records = candidate;
}

static bool action_move(const swan_frame_t *frame, uint8_t action) {
	uint16_t actions = (uint16_t)(frame->input->actions_pressed |
		frame->input->actions_repeated);
	return (actions & SWAN_GAME_ACTION_BIT(action)) != 0u;
}

static int8_t title_direction(const swan_frame_t *frame) {
	return (int8_t)action_move(frame, SWAN_ACTION_RIGHT) -
		(int8_t)action_move(frame, SWAN_ACTION_LEFT);
}

static void reset_hunt(void) {
	harpoon_reset(&state);
	if (!training_mode) {
		state.tutorial = HARPOON_TUTORIAL_COMPLETE;
		state.phase = HARPOON_PHASE_HUNT;
		state.resume_phase = HARPOON_PHASE_HUNT;
	}
}

void swan_game_boot(void) {
	harpoon_reset(&state);
	load_records();
	training_mode = records.tutorial_complete == 0u;
	harpoon_gfx_reset_title();
	start_title_music();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_INTRO) {
		load_records();
		training_mode = records.tutorial_complete == 0u;
		title_prompt = true;
		attract_mode = false;
		title_page = 0;
		title_idle_frames = 0;
		pause_audio_delay = 0;
		harpoon_gfx_reset_title();
		start_title_music();
	} else if (scene == SWAN_SCENE_HUNT) {
		reset_hunt();
		pause_audio_delay = 0;
		harpoon_gfx_init();
		swan_core_reset_session();
		start_hunt_music();
	} else if (scene == SWAN_SCENE_RESULT) {
		store_records(state.tutorial == HARPOON_TUTORIAL_COMPLETE, true);
		start_result_music();
	}
	swan_core_invalidate();
}

static void update_title(const swan_frame_t *frame) {
	bool prompt = ((frame->boot_tick / 30u) & 1u) == 0u;
	int8_t direction;
	if (attract_mode) {
		if (frame->input->pressed != 0u) {
			attract_mode = false;
			title_idle_frames = 0;
			play_effect(HARPOON_SFX_MOVE);
			swan_core_invalidate();
		}
		mark_trace(HARPOON_SFX_NONE);
		return;
	}
	if (frame->input->pressed != 0u) title_idle_frames = 0;
	else if (title_idle_frames < HARPOON_ATTRACT_FRAMES) ++title_idle_frames;
	if (prompt != title_prompt) {
		title_prompt = prompt;
		swan_core_invalidate();
	}
	direction = title_direction(frame);
	if (direction != 0) {
		if (direction > 0) title_page = (uint8_t)((title_page + 1u) % 3u);
		else title_page = title_page == 0u ? 2u : (uint8_t)(title_page - 1u);
		play_effect(HARPOON_SFX_MOVE);
		swan_core_invalidate();
	}
	if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_LURE)) {
		training_mode = !training_mode;
		play_effect(HARPOON_SFX_LURE);
		swan_core_invalidate();
	} else if (SWAN_GAME_ACTION_PRESSED(frame->input,
		SWAN_ACTION_START_OR_PAUSE)) {
		(void)swan_core_request_scene(SWAN_SCENE_HUNT, 0);
	} else if (title_idle_frames >= HARPOON_ATTRACT_FRAMES) {
		attract_mode = true;
		swan_core_invalidate();
	}
	mark_trace(HARPOON_SFX_NONE);
}

static void update_result(const swan_frame_t *frame) {
	if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_REPLAY)) {
		play_effect(HARPOON_SFX_RETRY);
		mark_trace(HARPOON_SFX_RETRY);
		(void)swan_core_request_scene(SWAN_SCENE_HUNT, 0);
	} else if (SWAN_GAME_ACTION_PRESSED(frame->input,
		SWAN_ACTION_START_OR_PAUSE)) {
		mark_trace(HARPOON_SFX_NONE);
		(void)swan_core_request_scene(SWAN_SCENE_INTRO, 0);
	} else {
		mark_trace(HARPOON_SFX_NONE);
	}
}

static void update_hunt(const swan_frame_t *frame) {
	harpoon_input_t input = {0};
	harpoon_event_t event;
	harpoon_phase_t previous_phase = state.phase;

	input.direction = (int8_t)action_move(frame, SWAN_ACTION_RIGHT) -
		(int8_t)action_move(frame, SWAN_ACTION_LEFT);
	input.charge_held = SWAN_GAME_ACTION_HELD(frame->input,
		SWAN_ACTION_CHARGE_FIRE);
	input.lure_held = SWAN_GAME_ACTION_HELD(frame->input, SWAN_ACTION_LURE);
	input.fire_released = SWAN_GAME_ACTION_RELEASED(frame->input,
		SWAN_ACTION_CHARGE_FIRE);
	input.pause = SWAN_GAME_ACTION_PRESSED(frame->input,
		SWAN_ACTION_START_OR_PAUSE);
	input.retry = state.phase == HARPOON_PHASE_PAUSED &&
		SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_RETRY);
	harpoon_step(&state, &input, frame->session_tick, &event);
	mark_trace(event.sfx);

	if (event.music == HARPOON_MUSIC_BOSS) start_boss_music();
	else if (event.music == HARPOON_MUSIC_HUNT) start_hunt_music();
	if (event.sfx == HARPOON_SFX_PAUSE) {
		if (state.phase == HARPOON_PHASE_PAUSED) {
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
	if (state.phase == HARPOON_PHASE_PAUSED && pause_audio_delay &&
		--pause_audio_delay == 0u) (void)swan_audio_pause();
	if (event.reset_session) {
		pause_audio_delay = 0;
		swan_core_reset_session();
		start_hunt_music();
		play_effect(HARPOON_SFX_RETRY);
	}
	if (state.tutorial == HARPOON_TUTORIAL_COMPLETE &&
		records.tutorial_complete == 0u)
		store_records(true, false);
	if (event.dirty || (state.phase != HARPOON_PHASE_PAUSED &&
		(frame->session_tick & 7u) == 0u)) swan_core_invalidate();
	if (previous_phase != HARPOON_PHASE_RESULT &&
		state.phase == HARPOON_PHASE_RESULT)
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	if (scene == SWAN_SCENE_INTRO) update_title(frame);
	else if (scene == SWAN_SCENE_RESULT) update_result(frame);
	else update_hunt(frame);
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		harpoon_gfx_show_title(title_prompt, attract_mode, title_page,
			training_mode, records.best_score, records.best_rank);
		return;
	}
	harpoon_gfx_render(&state, records.best_score, records.best_rank);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}

#undef mark_trace
