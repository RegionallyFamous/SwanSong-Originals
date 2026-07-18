#include <stdbool.h>
#include <stdint.h>

#include <swan/swan.h>

#include "swan_controls.h"
#include "swan_assets.h"
#include "swan_game_runtime.h"
#include "swan_project.h"
#include "gfx.h"
#include "model.h"

static mote_state_t state;
static swan_song_t SWAN_FAR active_song;

static const swan_audio_row_t SWAN_FAR *rows_for_track(uint8_t track) {
	if (track == 0) return swan_asset_track_0_rows;
	if (track == 1) return swan_asset_track_1_rows;
	return swan_asset_track_2_rows;
}

static void start_track(void) {
	active_song.rows = rows_for_track(state.track);
	active_song.row_count = 16;
	active_song.frames_per_row_q8 = (uint16_t)(state.tempo * 256u);
	active_song.loop = true;
	swan_audio_play_music(&active_song);
}

void swan_game_boot(void) {
	static const swan_gfx_config_t gfx_limits = {
		.tile_capacity = 512,
		.palette_capacity = 1,
		/* Zero means "use the default", so reserve one disabled slot. */
		.sprite_capacity = 1,
	};
	mote_reset(&state);
	swan_gfx_init(&gfx_limits);
	swan_gfx_set_sprites_enabled(false);
	swan_audio_init(swan_asset_track_0_instruments,
		SWAN_ASSET_TRACK_0_INSTRUMENT_COUNT);
}

void swan_scene_enter(swan_scene_id_t scene, uint16_t argument) {
	(void)argument;
	if (scene == SWAN_SCENE_TERMINAL) {
		gfx_init();
		swan_core_reset_session();
		start_track();
	}
	swan_core_invalidate();
}

void swan_scene_update(swan_scene_id_t scene, const swan_frame_t *frame) {
	mote_input_t model_input = {0};
	mote_event_t event;
	uint8_t previous_track;
	uint8_t previous_tempo;
	bool previous_playing;

	if (scene == SWAN_SCENE_INTRO) {
		if (swan_game_intro_complete(frame))
			(void)swan_core_request_scene(SWAN_SCENE_TERMINAL, 0);
		return;
	}

	model_input.track_direction =
		(int8_t)SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_NEXT_TRACK) -
		(int8_t)SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_PREVIOUS_TRACK);
	model_input.tempo_direction =
		(int8_t)SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_TEMPO_UP) -
		(int8_t)SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_TEMPO_DOWN);
	model_input.toggle_play = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_TOGGLE_PLAY);
	model_input.toggle_scope = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_TOGGLE_SCOPE);
	model_input.reset = SWAN_GAME_ACTION_PRESSED(frame->input, SWAN_ACTION_RESET);
	previous_track = state.track;
	previous_tempo = state.tempo;
	previous_playing = state.playing;
	mote_step(&state, &model_input, &event);
	if (event.reset_session) {
		swan_core_reset_session();
		gfx_init();
		start_track();
	} else {
		bool sequence_changed = state.track != previous_track ||
			state.tempo != previous_tempo;
		if (sequence_changed)
			start_track();
		if (!state.playing) {
			if (sequence_changed || previous_playing)
				(void)swan_audio_pause();
		} else if (!previous_playing && !sequence_changed) {
				if (!swan_audio_resume()) start_track();
		}
	}
	if (event.dirty) swan_core_invalidate();
}

void swan_scene_render(swan_scene_id_t scene) {
	if (scene == SWAN_SCENE_INTRO) {
		gfx_show_intro();
		return;
	}
	gfx_render(state.track, state.playing, state.tempo, state.scope, state.step,
		swan_audio_voices());
}

void swan_scene_exit(swan_scene_id_t scene) {
	(void)scene;
}
