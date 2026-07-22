#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <wonderful.h>
#include <ws/ports.h>

#include <swan/swan.h>

#include "swan_assets.h"
#include "swan_controls.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

#define RADIO_SAVE_SCHEMA 1u

typedef struct {
	uint16_t best_score;
	uint16_t best_time;
	uint8_t signals_discovered;
	uint8_t tutorial_complete;
	uint16_t reserved;
} radio_record_t;

_Static_assert(sizeof(radio_record_t) == 8u,
	"Radio Ghost save record must remain byte-stable");

static const swan_audio_sfx_policy_t radio_sfx_policy = {
	.preferred_channel = 3,
	.reserved_channel_mask = 0,
	.music_steal_channel_mask = (uint8_t)(1u << 3),
	.music_duck_volume = 11,
	.music_priority = {10, 8, 6, 0},
};

static radio_state_t state;
static bool title_prompt = true;
static uint8_t pause_audio_delay;
static int8_t pan_channel = -1;
static int8_t pan_direction;
static uint8_t pan_frames;
static swan_ws_sram_context_t sram_context;
static swan_storage_t cartridge_storage;
static radio_record_t records;
static bool storage_ready;

#define ACTION_BIT(action) ((uint16_t)(1u << (action)))
#define ACTION_REPEATED(input, action) \
	(((input)->actions_repeated & ACTION_BIT(action)) != 0u)

#if SWAN_DETERMINISTIC_TRACE
static void trace_hash_u8(uint32_t *hash, uint8_t value) {
	*hash = (*hash ^ value) * 16777619u;
}

static void trace_hash_u16(uint32_t *hash, uint16_t value) {
	trace_hash_u8(hash, (uint8_t)value);
	trace_hash_u8(hash, (uint8_t)(value >> 8));
}

static uint32_t trace_state_hash(void) {
	uint32_t hash = radio_state_hash(&state);
	trace_hash_u16(&hash, records.best_score);
	trace_hash_u16(&hash, records.best_time);
	trace_hash_u8(&hash, records.signals_discovered);
	trace_hash_u8(&hash, records.tutorial_complete);
	trace_hash_u16(&hash, records.reserved);
	trace_hash_u8(&hash, storage_ready ? 1u : 0u);
	return hash;
}

static void mark_trace(radio_sfx_t effect) {
	swan_debug_frame_mark_state(radio_state_progress(&state),
		trace_state_hash());
	swan_debug_frame_mark_ending((uint8_t)state.result);
	if (effect != RADIO_SFX_NONE)
		swan_debug_frame_mark_audio((uint16_t)(1u << ((uint8_t)effect - 1u)));
}
#else
#define mark_trace(effect) ((void)0)
#endif

static const swan_sfx_t SWAN_FAR *effect_for(radio_sfx_t effect) {
	switch (effect) {
		case RADIO_SFX_TUNE_LEFT: return &swan_asset_tune_left_sfx;
		case RADIO_SFX_TUNE_RIGHT: return &swan_asset_tune_right_sfx;
		case RADIO_SFX_GAIN: return &swan_asset_gain_sfx;
		case RADIO_SFX_GATE: return &swan_asset_gate_sfx;
		case RADIO_SFX_CONFIRM: return &swan_asset_confirm_sfx;
		case RADIO_SFX_LOCK: return &swan_asset_lock_sfx;
		case RADIO_SFX_ERROR: return &swan_asset_error_sfx;
		case RADIO_SFX_COMPLETE: return &swan_asset_complete_sfx;
		case RADIO_SFX_FAILURE: return &swan_asset_failure_sfx;
		case RADIO_SFX_PAUSE: return &swan_asset_pause_sfx;
		default: return 0;
	}
}

static uint8_t effect_frames(radio_sfx_t effect) {
	switch (effect) {
		case RADIO_SFX_COMPLETE: return 28u;
		case RADIO_SFX_FAILURE: return 25u;
		case RADIO_SFX_LOCK: return 15u;
		case RADIO_SFX_ERROR: return 13u;
		case RADIO_SFX_GATE: return 9u;
		case RADIO_SFX_CONFIRM: return 10u;
		default: return 7u;
	}
}

static void play_effect(radio_sfx_t effect, int8_t pan) {
	const swan_sfx_t SWAN_FAR *sfx = effect_for(effect);
	int8_t channel;
	if (sfx == 0) return;
	channel = swan_audio_play_sfx(sfx);
	if (channel >= 0) {
		pan_channel = channel;
		pan_direction = pan;
		pan_frames = effect_frames(effect);
	}
}

/*
 * SwanSong SDK v0.5 exposes deterministic voice volume but not stereo pan.
 * Keep this small, game-local Wonderful register shim until the public audio
 * API grows pan. Pitch and on-screen direction remain complete mono fallbacks.
 */
static void apply_spatial_pan(void) {
	static const uint8_t volume_ports[4] = {
		WS_SOUND_VOL_CH1_PORT, WS_SOUND_VOL_CH2_PORT,
		WS_SOUND_VOL_CH3_PORT, WS_SOUND_VOL_CH4_PORT,
	};
	const swan_audio_voice_t *voices;
	uint8_t volume;
	uint8_t quiet;
	uint8_t left;
	uint8_t right;
	if (pan_channel < 0 || pan_channel >= 4 || pan_frames == 0u) return;
	voices = swan_audio_voices();
	volume = voices[(uint8_t)pan_channel].volume;
	quiet = volume > 3u ? (uint8_t)(volume / 3u) : volume;
	left = pan_direction < 0 ? volume : pan_direction > 0 ? quiet : volume;
	right = pan_direction > 0 ? volume : pan_direction < 0 ? quiet : volume;
	outportb(volume_ports[(uint8_t)pan_channel],
		(uint8_t)((left << 4) | right));
	--pan_frames;
	if (pan_frames == 0u) pan_channel = -1;
}

static void start_title_music(void) {
	swan_audio_init(swan_asset_title_song_instruments,
		SWAN_ASSET_TITLE_SONG_INSTRUMENT_COUNT);
	swan_audio_set_sfx_policy(&radio_sfx_policy);
	swan_audio_play_music(&swan_asset_title_song_song);
}

static void start_night_music(void) {
	swan_audio_init(swan_asset_night_song_instruments,
		SWAN_ASSET_NIGHT_SONG_INSTRUMENT_COUNT);
	swan_audio_set_sfx_policy(&radio_sfx_policy);
	swan_audio_play_music(&swan_asset_night_song_song);
}

static int8_t action_axis(const swan_input_t *input,
	uint8_t negative, uint8_t positive) {
	return (int8_t)ACTION_REPEATED(input, positive) -
		(int8_t)ACTION_REPEATED(input, negative);
}

static void show_records(void) {
	gfx_set_records(records.best_score, records.best_time,
		records.signals_discovered, records.tutorial_complete != 0u);
}

static void load_records(void) {
	swan_save_info_t info;
	swan_save_status_t status;
	memset(&records, 0, sizeof(records));
	storage_ready = swan_ws_sram_storage(&sram_context,
		&cartridge_storage, 8192ul);
	if (storage_ready) {
		status = swan_save_load(&cartridge_storage, RADIO_SAVE_SCHEMA,
			&records, sizeof(records), &info);
		if (status != SWAN_SAVE_OK ||
			records.signals_discovered > RADIO_SIGNAL_COUNT ||
			records.tutorial_complete > 1u)
			memset(&records, 0, sizeof(records));
	}
	show_records();
}

static void store_records(bool tutorial_complete) {
	radio_record_t candidate = records;
	swan_save_info_t info;
	bool changed = false;
	if (tutorial_complete && candidate.tutorial_complete == 0u) {
		candidate.tutorial_complete = 1u;
		changed = true;
	}
	if (state.clue > candidate.signals_discovered) {
		candidate.signals_discovered = state.clue;
		changed = true;
	}
	if (state.result == RADIO_RESULT_SIGNAL) {
		if (state.score > candidate.best_score) {
			candidate.best_score = state.score;
			changed = true;
		}
		if (state.time > candidate.best_time) {
			candidate.best_time = state.time;
			changed = true;
		}
	}
	if (changed && storage_ready &&
		swan_save_store(&cartridge_storage, RADIO_SAVE_SCHEMA,
			&candidate, sizeof(candidate), &info) == SWAN_SAVE_OK)
		records = candidate;
	show_records();
}

void swan_game_boot(void) {
	radio_reset_title(&state);
	gfx_reset_title();
	pan_channel = -1;
	pan_frames = 0;
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	if (scene == SWAN_SCENE_TITLE) {
		radio_reset_title(&state);
		load_records();
		gfx_reset_title();
		title_prompt = true;
		pause_audio_delay = 0u;
		pan_channel = -1;
		pan_frames = 0u;
		start_title_music();
	} else if (scene == SWAN_SCENE_TUTORIAL) {
		radio_reset_tutorial(&state);
		swan_core_reset_session();
		gfx_init();
		start_night_music();
		play_effect(RADIO_SFX_CONFIRM, 0);
	} else if (scene == SWAN_SCENE_TUNING) {
		if (argument == 0u) {
			radio_reset_night(&state);
			swan_core_reset_session();
			gfx_init();
			start_night_music();
			play_effect(RADIO_SFX_CONFIRM, 0);
		} else {
			radio_set_paused(&state, false);
			pause_audio_delay = 0u;
			(void)swan_audio_resume();
			play_effect(RADIO_SFX_PAUSE, 0);
		}
	} else if (scene == SWAN_SCENE_PAUSE) {
		pause_audio_delay = 4u;
	} else if (scene == SWAN_SCENE_RESULT) {
		store_records(false);
	}
	swan_core_invalidate();
}

static void handle_title(const swan_frame_t *frame) {
	radio_input_t input = {0};
	radio_event_t event;
	bool prompt;
	input.menu_direction = action_axis(frame->input,
		SWAN_ACTION_TUNE_DOWN, SWAN_ACTION_TUNE_UP);
	input.confirm = SWAN_GAME_ACTION_PRESSED(frame->input,
		SWAN_ACTION_CONFIRM_TITLE);
	radio_step(&state, &input, &event);
	prompt = ((state.title_ticks / 30u) & 1u) == 0u;
	if (prompt != title_prompt) {
		title_prompt = prompt;
		event.dirty = true;
	}
	play_effect(event.sfx, event.pan);
	mark_trace(event.sfx);
	if (event.start_tutorial)
		(void)swan_core_request_scene(SWAN_SCENE_TUTORIAL, 0);
	else if (event.start_night)
		(void)swan_core_request_scene(SWAN_SCENE_TUNING, 0);
	if (event.dirty) swan_core_invalidate();
}

static void handle_receiver(const swan_frame_t *frame) {
	radio_input_t input = {0};
	radio_event_t event;
	input.frequency_direction = action_axis(frame->input,
		SWAN_ACTION_TUNE_DOWN, SWAN_ACTION_TUNE_UP);
	input.gain_direction = action_axis(frame->input,
		SWAN_ACTION_GAIN_DOWN, SWAN_ACTION_GAIN_UP);
	input.lock = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_LOCK_SIGNAL);
	input.toggle_gate = SWAN_GAME_ACTION_PRESSED(frame->input,
		SWAN_ACTION_TOGGLE_GATE);
	input.pause = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_PAUSE);
	radio_step(&state, &input, &event);
	play_effect(event.sfx, event.pan);
	mark_trace(event.sfx);
	if (event.start_night) {
		if (state.mode == RADIO_MODE_TUTORIAL) store_records(true);
		(void)swan_core_request_scene(SWAN_SCENE_TUNING, 0);
	} else if (event.pause_changed && state.paused)
		(void)swan_core_request_scene(SWAN_SCENE_PAUSE, 0);
	else if (event.show_result)
		(void)swan_core_request_scene(SWAN_SCENE_RESULT, 0);
	if (event.dirty || (state.result == RADIO_RESULT_PLAYING &&
		(frame->session_tick & 7u) == 0u)) swan_core_invalidate();
}

static void handle_pause(const swan_frame_t *frame) {
	if (pause_audio_delay != 0u && --pause_audio_delay == 0u) {
		pan_channel = -1;
		pan_frames = 0u;
		(void)swan_audio_pause();
	}
	if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_PAUSE)) {
		radio_input_t input = {0};
		radio_event_t event;
		(void)swan_audio_resume();
		input.pause = true;
		radio_step(&state, &input, &event);
		mark_trace(event.sfx);
		(void)swan_core_request_scene(SWAN_SCENE_TUNING, 1);
	} else if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_LOCK_SIGNAL)) {
		(void)swan_audio_resume();
		play_effect(RADIO_SFX_CONFIRM, 0);
		mark_trace(RADIO_SFX_CONFIRM);
		(void)swan_core_request_scene(SWAN_SCENE_TUNING, 0);
	} else if (SWAN_GAME_ACTION_PRESSED(frame->input,
		SWAN_ACTION_TOGGLE_GATE)) {
		(void)swan_audio_resume();
		mark_trace(RADIO_SFX_CONFIRM);
		(void)swan_core_request_scene(SWAN_SCENE_TITLE, 0);
	} else {
		mark_trace(RADIO_SFX_NONE);
	}
}

static void handle_result(const swan_frame_t *frame) {
	if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_LOCK_SIGNAL)) {
		mark_trace(RADIO_SFX_CONFIRM);
		(void)swan_core_request_scene(SWAN_SCENE_TUNING, 0);
	} else if (SWAN_GAME_ACTION_PRESSED(frame->input,
		SWAN_ACTION_TOGGLE_GATE)) {
		mark_trace(RADIO_SFX_CONFIRM);
		(void)swan_core_request_scene(SWAN_SCENE_TUTORIAL, 0);
	} else if (SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_PAUSE)) {
		mark_trace(RADIO_SFX_CONFIRM);
		(void)swan_core_request_scene(SWAN_SCENE_TITLE, 0);
	} else {
		mark_trace(RADIO_SFX_NONE);
	}
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	if (scene == SWAN_SCENE_TITLE) handle_title(frame);
	else if (scene == SWAN_SCENE_TUTORIAL || scene == SWAN_SCENE_TUNING)
		handle_receiver(frame);
	else if (scene == SWAN_SCENE_PAUSE) handle_pause(frame);
	else handle_result(frame);
	apply_spatial_pan();
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_TITLE) {
		gfx_show_title(&state, title_prompt);
		return;
	}
	gfx_render(&state);
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}

#undef ACTION_REPEATED
#undef ACTION_BIT
#undef mark_trace
