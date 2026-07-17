#ifndef SWANSONG_GAME_RUNTIME_H
#define SWANSONG_GAME_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

#include <swan/input.h>
#include <swan/types.h>

struct swan_frame;

#define SWAN_GAME_ACTION_BIT(action) ((uint16_t)(1u << (action)))
#define SWAN_GAME_ACTION_PRESSED(input, action) \
	(((input)->actions_pressed & SWAN_GAME_ACTION_BIT(action)) != 0)
#define SWAN_GAME_ACTION_HELD(input, action) \
	(((input)->actions_held & SWAN_GAME_ACTION_BIT(action)) != 0)
#define SWAN_GAME_ACTION_RELEASED(input, action) \
	(((input)->actions_released & SWAN_GAME_ACTION_BIT(action)) != 0)

int8_t swan_game_primary_axis(uint16_t keys);
bool swan_game_intro_complete(const struct swan_frame *frame);

void swan_game_audio_init(void);
void swan_game_audio_beep(uint16_t hz, uint8_t duration_frames);
void swan_game_audio_tone(uint16_t hz, uint8_t volume);
void swan_game_audio_off(void);

void swan_game_gfx_show_intro(const uint8_t SWAN_FAR *tiles,
	uint16_t tile_bytes, const uint16_t SWAN_FAR *tilemap,
	const uint16_t SWAN_FAR *palette);

#endif
