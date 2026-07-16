#ifndef RF_SWAN_H
#define RF_SWAN_H

#include <stdbool.h>
#include <stdint.h>
#include <wonderful.h>
#include <ws.h>

typedef struct {
	uint16_t held;
	uint16_t pressed;
	uint16_t released;
} rf_input_t;

void rf_init(bool vertical);
void rf_set_orientation(bool vertical);
void rf_frame(void);
void rf_clear(void);
void rf_header(const char __far *title, const char __far *subtitle);
void rf_footer(const char __far *help);
void rf_art_load(const uint8_t __far *tiles, uint16_t tile_bytes,
	const uint16_t __far *tilemap, uint8_t width, uint8_t height,
	uint8_t screen_x, uint8_t screen_y, const uint16_t __far *palette);
void rf_playfield_begin(void);
void rf_playfield_end(void);

const rf_input_t *rf_input(void);
int8_t rf_dx(uint16_t keys);
int8_t rf_dy(uint16_t keys);
bool rf_pressed_any_direction(void);

uint16_t rf_frame_count(void);
uint16_t rf_random(void);
uint8_t rf_clamp_u8(int16_t value, uint8_t low, uint8_t high);
void rf_print_bar(uint8_t value, uint8_t maximum, uint8_t width);

void rf_tone(uint16_t hz, uint8_t volume);
void rf_sound_off(void);
void rf_beep(uint16_t hz, uint8_t duration_frames);

#endif
