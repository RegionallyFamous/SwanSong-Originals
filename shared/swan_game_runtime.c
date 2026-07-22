#include <swan/audio.h>
#include <swan/core.h>
#include <swan/gfx.h>
#include <swan/input.h>

#include "swan_game_runtime.h"

static const swan_instrument_t SWAN_FAR feedback_instrument = {
	.wave = {0, 1, 3, 6, 10, 13, 15, 13, 10, 6, 3, 1, 0, 1, 0, 1},
	.attack = 0,
	.release = 0,
};

static swan_sfx_step_t SWAN_FAR feedback_steps[2] = {
	{{36, 0, 8}, 1},
	{{SWAN_AUDIO_NOTE_OFF, SWAN_AUDIO_NO_CHANGE, 0}, 1},
};
static swan_sfx_t SWAN_FAR feedback_effect = {
	feedback_steps, 2, 8
};

static uint16_t note_frequency(uint8_t note) {
	static const uint8_t base_hz[12] = {
		65, 69, 73, 78, 82, 87, 92, 98, 104, 110, 117, 123
	};
	uint8_t octave = (uint8_t)(note / 12u);
	if (octave > 5) octave = 5;
	return (uint16_t)base_hz[note % 12u] << octave;
}

static uint8_t nearest_note(uint16_t hz) {
	uint16_t best_distance = UINT16_MAX;
	uint8_t best = 0;
	uint8_t note;
	for (note = 0; note < 72; ++note) {
		uint16_t candidate = note_frequency(note);
		uint16_t distance = candidate > hz ? candidate - hz : hz - candidate;
		if (distance < best_distance) {
			best = note;
			best_distance = distance;
		}
	}
	return best;
}

static void play_feedback(uint16_t hz, uint8_t volume, uint8_t duration_frames) {
	feedback_steps[0].command.note = nearest_note(hz);
	feedback_steps[0].command.instrument = 0;
	feedback_steps[0].command.volume = volume > 15 ? 15 : volume;
	feedback_steps[0].duration_frames = duration_frames == 0 ? 1 : duration_frames;
	(void)swan_audio_play_sfx(&feedback_effect);
}

int8_t swan_game_primary_axis(uint16_t keys) {
	int8_t value = swan_input_dx(keys);
	return value != 0 ? value : swan_input_dy(keys);
}

bool swan_game_intro_complete(const swan_frame_t *frame) {
	return frame->boot_tick >= 36u || frame->input->pressed != 0;
}

void swan_game_audio_init(void) {
	swan_audio_sfx_policy_t policy = SWAN_AUDIO_SFX_POLICY_DEFAULT;
	swan_audio_init(&feedback_instrument, 1);
	policy.preferred_channel = 3;
	policy.reserved_channel_mask = (uint8_t)(1u << 3);
	swan_audio_set_sfx_policy(&policy);
}

void swan_game_audio_beep(uint16_t hz, uint8_t duration_frames) {
	play_feedback(hz, 6, duration_frames);
}

void swan_game_audio_tone(uint16_t hz, uint8_t volume) {
	play_feedback(hz, volume, UINT8_MAX);
}

void swan_game_audio_off(void) {
	swan_audio_stop_all();
}

void swan_game_gfx_show_intro(const uint8_t SWAN_FAR *tiles,
	uint16_t tile_bytes, const uint16_t SWAN_FAR *tilemap,
	const uint16_t SWAN_FAR *palette) {
	uint16_t local_palette[4];
	uint8_t x;
	uint8_t y;

	/* Approved intro maps are one-based, matching their legacy tile upload. */
	(void)swan_gfx_load_tiles(1, tiles, (uint16_t)(tile_bytes / 16u));
	for (x = 0; x < 4; ++x) local_palette[x] = palette[x];
	(void)swan_gfx_set_palette(0, local_palette);
	(void)swan_gfx_fill(0, 0, 0, 32, 32, SWAN_TILE_ATTR(0, 0));
	for (y = 0; y < 18; ++y) {
		for (x = 0; x < 28; ++x) {
			(void)swan_gfx_put_tile(0, x, y,
				tilemap[(uint16_t)y * 28u + x]);
		}
	}
	swan_gfx_set_scroll(0, 0, 0);
	swan_gfx_set_layer_enabled(0, true);
	swan_gfx_set_layer_enabled(1, false);
}
