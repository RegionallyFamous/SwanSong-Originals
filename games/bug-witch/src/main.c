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

#define BUG_SAVE_SCHEMA 1u
#define BUG_ATTRACT_FRAMES 755u

typedef struct {
	uint8_t best_medals;
	uint8_t completed_spellbooks;
	uint8_t tutorial_learned;
	uint8_t reserved;
} bug_record_t;

_Static_assert(sizeof(bug_record_t) == 4u,
	"Bug Witch save record must remain byte-stable");

static const swan_audio_sfx_policy_t bug_sfx_policy = {
	.preferred_channel = 3,
	.reserved_channel_mask = 0,
	.music_steal_channel_mask = (uint8_t)(1u << 3),
	.music_duck_volume = 12,
	.music_priority = {10, 8, 6, 0},
};

static bug_state_t state;
static uint8_t title_option;
static bool title_prompt = true;
static bool tutorial_done;
static bool attract_mode;
static uint8_t pause_audio_delay;
static uint16_t title_idle_frames;
static swan_ws_eeprom_context_t eeprom_context;
static swan_storage_t cartridge_storage;
static bug_record_t records;
static bool storage_ready;

#if SWAN_DETERMINISTIC_TRACE
static void mark_trace(bug_sfx_t effect) {
	uint8_t ending = state.complete ? 1u : tutorial_done ? 2u : 0u;
	swan_debug_frame_mark_state(bug_progress(&state), bug_state_hash(&state));
	swan_debug_frame_mark_ending(ending);
	if (effect != BUG_SFX_NONE)
		swan_debug_frame_mark_audio(
			(uint16_t)(1u << ((uint8_t)effect - 1u)));
}
#else
#define mark_trace(effect) ((void)0)
#endif

static const swan_sfx_t SWAN_FAR *effect_for(bug_sfx_t effect) {
	switch (effect) {
		case BUG_SFX_MOVE: return &swan_asset_move_sfx;
		case BUG_SFX_SELECT: return &swan_asset_select_sfx;
		case BUG_SFX_PLACE: return &swan_asset_place_sfx;
		case BUG_SFX_CLEAR: return &swan_asset_clear_sfx;
		case BUG_SFX_UNDO: return &swan_asset_undo_sfx;
		case BUG_SFX_CAST: return &swan_asset_cast_sfx;
		case BUG_SFX_FAIL: return &swan_asset_fail_sfx;
		case BUG_SFX_HINT: return &swan_asset_hint_sfx;
		case BUG_SFX_SOLVE: return &swan_asset_solve_sfx;
		case BUG_SFX_FINISH: return &swan_asset_finish_sfx;
		case BUG_SFX_PAUSE: return &swan_asset_pause_sfx;
		default: return 0;
	}
}

static void play_effect(bug_sfx_t effect) {
	const swan_sfx_t SWAN_FAR *sfx = effect_for(effect);
	if (sfx) (void)swan_audio_play_sfx(sfx);
}

static void start_title_music(void) {
	swan_audio_init(swan_asset_title_song_instruments,
		SWAN_ASSET_TITLE_SONG_INSTRUMENT_COUNT);
	swan_audio_set_sfx_policy(&bug_sfx_policy);
	swan_audio_play_music(&swan_asset_title_song_song);
}

static void start_game_music(void) {
	swan_audio_init(swan_asset_circuit_song_instruments,
		SWAN_ASSET_CIRCUIT_SONG_INSTRUMENT_COUNT);
	swan_audio_set_sfx_policy(&bug_sfx_policy);
	swan_audio_play_music(&swan_asset_circuit_song_song);
}

static void load_records(void) {
	swan_save_info_t info;
	swan_save_status_t status;
	memset(&records, 0, sizeof(records));
	storage_ready = swan_ws_eeprom_storage(&eeprom_context,
		&cartridge_storage, 128u);
	if (!storage_ready) return;
	status = swan_save_load(&cartridge_storage, BUG_SAVE_SCHEMA,
		&records, sizeof(records), &info);
	if (status != SWAN_SAVE_OK || records.best_medals > 15u ||
		records.tutorial_learned > 1u)
		memset(&records, 0, sizeof(records));
}

static void store_records(bool learned, bool completed) {
	bug_record_t candidate = records;
	swan_save_info_t info;
	bool changed = false;
	if (learned && candidate.tutorial_learned == 0u) {
		candidate.tutorial_learned = 1u;
		changed = true;
	}
	if (completed) {
		if (candidate.completed_spellbooks != UINT8_MAX)
			++candidate.completed_spellbooks;
		if (state.total_medals > candidate.best_medals)
			candidate.best_medals = state.total_medals;
		changed = true;
	}
	if (changed && storage_ready &&
		swan_save_store(&cartridge_storage, BUG_SAVE_SCHEMA,
			&candidate, sizeof(candidate), &info) == SWAN_SAVE_OK)
		records = candidate;
}

void swan_game_boot(void) {
	bug_reset(&state);
	load_records();
	start_title_music();
	gfx_reset_title();
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	if (scene == SWAN_SCENE_INTRO) {
		bug_reset(&state);
		load_records();
		title_option = 0;
		title_prompt = true;
		tutorial_done = false;
		attract_mode = false;
		pause_audio_delay = 0;
		title_idle_frames = 0;
		gfx_reset_title();
		start_title_music();
	} else if (scene == SWAN_SCENE_CIRCUIT ||
		scene == SWAN_SCENE_SPELLBOOK) {
		bug_reset(&state);
		tutorial_done = false;
		attract_mode = scene == SWAN_SCENE_SPELLBOOK && argument == 1u;
		pause_audio_delay = 0;
		gfx_init();
		swan_core_reset_session();
		start_game_music();
	}
	swan_core_invalidate();
}

static void update_title(const swan_frame_t *frame) {
	int8_t direction = swan_input_dx(frame->input->pressed);
	bool prompt = ((frame->boot_tick / 30u) & 1u) == 0;
	if (frame->input->pressed != 0u) title_idle_frames = 0;
	else if (title_idle_frames < BUG_ATTRACT_FRAMES) ++title_idle_frames;
	if (direction == 0) direction = swan_input_dy(frame->input->pressed);
	if (direction != 0) {
		title_option ^= 1u;
		play_effect(BUG_SFX_MOVE);
		swan_core_invalidate();
	}
	if (prompt != title_prompt) {
		title_prompt = prompt;
		swan_core_invalidate();
	}
	if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_RUN_CIRCUIT)) {
		(void)swan_core_request_scene(SWAN_SCENE_CIRCUIT, 0);
	} else if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_PLACE)) {
		play_effect(BUG_SFX_SELECT);
		(void)swan_core_request_scene(title_option == 0 ?
			SWAN_SCENE_CIRCUIT : SWAN_SCENE_SPELLBOOK, 0);
	} else if (title_idle_frames >= BUG_ATTRACT_FRAMES) {
		(void)swan_core_request_scene(SWAN_SCENE_SPELLBOOK, 1);
	}
	mark_trace(BUG_SFX_NONE);
}

static void update_result(const swan_frame_t *frame) {
	if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_PLACE)) {
		(void)swan_core_request_scene(SWAN_SCENE_CIRCUIT, 0);
	} else if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_CLEAR) ||
		SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_RUN_CIRCUIT)) {
		(void)swan_core_request_scene(SWAN_SCENE_INTRO, 0);
	}
	mark_trace(BUG_SFX_NONE);
}

static void map_game_input(const swan_frame_t *frame, bug_input_t *input) {
	bool place = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_PLACE);
	bool clear = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_CLEAR);
	bool pause = place && clear;
	input->pause = pause;
	if (state.phase == BUG_PHASE_PAUSED) {
		input->place = place && !pause;
		input->retry = clear && !place;
		input->return_title = SWAN_GAME_ACTION_PRESSED(frame->input,
			SWAN_ACTION_RUN_CIRCUIT);
		return;
	}
	input->cursor_direction = swan_input_dx(frame->input->pressed);
	input->selection_direction = swan_input_dy(frame->input->pressed);
	input->place = place && !pause;
	input->clear = clear && !pause;
	input->run = SWAN_GAME_ACTION_PRESSED(frame->input,
		SWAN_ACTION_RUN_CIRCUIT);
}

static void handle_audio_event(bug_phase_t previous_phase,
	const bug_event_t *event) {
	if (event->sfx == BUG_SFX_PAUSE) {
		if (state.phase == BUG_PHASE_PAUSED) {
			play_effect(event->sfx);
			pause_audio_delay = 3;
		} else {
			pause_audio_delay = 0;
			(void)swan_audio_resume();
			play_effect(event->sfx);
		}
	} else {
		if (previous_phase == BUG_PHASE_PAUSED &&
			state.phase != BUG_PHASE_PAUSED)
			(void)swan_audio_resume();
		play_effect(event->sfx);
	}
}

static void update_game(swan_scene_id_t scene, const swan_frame_t *frame) {
	bug_input_t input = {0};
	bug_event_t event;
	bug_phase_t previous_phase = state.phase;

	if (attract_mode && frame->input->pressed != 0u) {
		(void)swan_core_request_scene(SWAN_SCENE_INTRO, 0);
		mark_trace(BUG_SFX_NONE);
		return;
	}
	if (scene == SWAN_SCENE_SPELLBOOK && tutorial_done) {
		if (attract_mode && frame->session_tick >= 150u) {
			(void)swan_core_request_scene(SWAN_SCENE_INTRO, 0);
			mark_trace(BUG_SFX_NONE);
			return;
		}
		if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_PLACE) ||
			SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_RUN_CIRCUIT))
			(void)swan_core_request_scene(SWAN_SCENE_INTRO, 0);
		else if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_CLEAR)) {
			bug_reset(&state);
			tutorial_done = false;
			swan_core_invalidate();
		}
		mark_trace(BUG_SFX_NONE);
		return;
	}

	if (attract_mode) {
		input.place = frame->session_tick == 30u;
		input.run = frame->session_tick == 60u;
	} else {
		map_game_input(frame, &input);
	}
	bug_step(&state, &input, &event);
	if (scene == SWAN_SCENE_SPELLBOOK && event.solved) {
		tutorial_done = true;
		if (!attract_mode) store_records(true, false);
		event.dirty = true;
	}
	handle_audio_event(previous_phase, &event);
	if (state.phase == BUG_PHASE_PAUSED && pause_audio_delay &&
		--pause_audio_delay == 0) (void)swan_audio_pause();
	if (event.dirty) swan_core_invalidate();
	mark_trace(event.sfx);
	if (event.returned_title) {
		(void)swan_core_request_scene(SWAN_SCENE_INTRO, 0);
	} else if (scene == SWAN_SCENE_CIRCUIT && event.completed) {
		store_records(false, true);
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
	}
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	if (scene == SWAN_SCENE_INTRO) update_title(frame);
	else if (scene == SWAN_SCENE_RESULT) update_result(frame);
	else update_game(scene, frame);
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		gfx_show_intro(title_option, title_prompt, records.best_medals,
			records.tutorial_learned != 0u);
		return;
	}
	gfx_render(&state, scene == SWAN_SCENE_SPELLBOOK, tutorial_done,
		swan_core_session_tick(), records.best_medals);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}

#undef mark_trace
