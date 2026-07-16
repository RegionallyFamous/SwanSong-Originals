#include <stdio.h>
#include <string.h>
#include <wonderful.h>
#include <ws.h>
#include <wse/memory.h>
#include <wsx/console.h>

#include "rf_swan.h"

WSE_RESERVE_TILES(512, 0);

static rf_input_t input_state;
static uint16_t frame_counter;
static uint16_t random_state = 0x71D3;
static uint8_t tone_frames;
static uint8_t text_x;
static uint8_t text_y;
static bool color_active;
static bool playfield_mode;
static const uint16_t __far *active_art_map;
static uint8_t active_art_width;
static uint8_t active_art_height;
static uint8_t active_art_x;
static uint8_t active_art_y;
static ws_sound_wavetable_t sound_waves __attribute__((aligned(64)));

static const char __far rule[] = "==//====================//==";

#define RF_ICON_BASE 392

static const ws_display_tile_t __far icon_tiles[] = {
	/* wall: broken photocopy ink */
	{ .row = {0x00FF, 0x00DB, 0x00FF, 0x00BD, 0x00FF, 0x00E7, 0x00FF, 0x00BD} },
	/* floor: offset blue/pink registration dots */
	{ .row = {0x0000, 0x0400, 0x0000, 0x2000, 0x0000, 0x0200, 0x0000, 0x1000} },
	/* player: accent-2 diamond */
	{ .row = {0x0000, 0x1818, 0x3C3C, 0x7E7E, 0xFFFF, 0x7E7E, 0x3C3C, 0x1818} },
	/* item: accent-1 taped parcel */
	{ .row = {0x0000, 0x7E00, 0x4200, 0x5A00, 0x5A00, 0x4200, 0x7E00, 0x0000} },
	/* goal: two-ink ring */
	{ .row = {0x1818, 0x3C24, 0x6642, 0xC381, 0xC381, 0x6642, 0x3C24, 0x1818} },
	/* selected: editorial corner brackets */
	{ .row = {0xE7E7, 0x8181, 0x0000, 0x0000, 0x0000, 0x0000, 0x8181, 0xE7E7} },
	/* ally: accent-1 chevron */
	{ .row = {0x1800, 0x3C00, 0x7E00, 0xDB00, 0x1800, 0x3C00, 0x6600, 0xC300} },
	/* enemy: black cut-paper wedge */
	{ .row = {0x0018, 0x003C, 0x007E, 0x00FF, 0x00FF, 0x007E, 0x003C, 0x0018} }
};

static uint16_t icon_attr(uint8_t c) {
	uint16_t tile = 0;
	switch (c) {
		case '#': tile = RF_ICON_BASE; break;
		case '.': case '~': case '_': tile = RF_ICON_BASE + 1; break;
		case '@': case 'C': case 'S': tile = RF_ICON_BASE + 2; break;
		case 'P': case 'K': case '*': case 'M': tile = RF_ICON_BASE + 3; break;
		case 'D': case 'G': case 'B': tile = RF_ICON_BASE + 4; break;
		case '+': case '^': tile = RF_ICON_BASE + 5; break;
		case 'A': tile = RF_ICON_BASE + 6; break;
		case 'E': tile = RF_ICON_BASE + 7; break;
		default: return 0;
	}
	return WS_SCREEN_ATTR_TILE(tile) | WS_SCREEN_ATTR_PALETTE(1);
}

static int fixed_console_put(uint8_t c, FILE *stream) {
	uint16_t attr;
	(void)stream;
	if (c == '\r') {
		text_x = 0;
		return c;
	}
	if (c == '\n') {
		text_x = 0;
		if (text_y < 18) ++text_y;
		return c;
	}
	if (c == '\t') {
		text_x = (uint8_t)((text_x + 4) & (uint8_t)~3);
		return c;
	}
	if (text_x >= 28) {
		text_x = 0;
		if (text_y < 18) ++text_y;
	}
	if (text_y >= 18) return c;
	if (c < 0x20 || c >= 0x80) c = '?';
	attr = playfield_mode ? icon_attr(c) : 0;
	if (!attr) {
		uint8_t palette = 0;
		if (text_y == 0) palette = 2;
		else if (c == '=' || c == '/') palette = 3;
		attr = WS_SCREEN_ATTR_TILE(0x1A0 + c - 0x20) |
			WS_SCREEN_ATTR_PALETTE(palette);
	}
	wse_screen1.row[text_y].cell[text_x] = attr;
	++text_x;
	return c;
}

static const struct wf_fileops_t __far fixed_console_ops = {
	.get = 0,
	.put = fixed_console_put,
	.close = 0
};

static FILE fixed_console_file = {
	.ops = &fixed_console_ops,
	.flags = 0
};

static void init_sound(void) {
	uint8_t wave;
	uint8_t sample;

	ws_sound_reset();
	for (wave = 0; wave < 4; ++wave) {
		for (sample = 0; sample < 16; ++sample) {
			sound_waves.wave[wave].data[sample] =
				(sample < 8) ? (uint8_t)(0xED - sample) : (uint8_t)(0x12 + sample);
		}
	}
	ws_sound_set_wavetable_address(&sound_waves);
	outportb(WS_SOUND_OUT_CTRL_PORT,
		WS_SOUND_OUT_CTRL_SPEAKER_ENABLE |
		WS_SOUND_OUT_CTRL_HEADPHONE_ENABLE |
		WS_SOUND_OUT_CTRL_SPEAKER_VOLUME_100);
}

void rf_init(bool vertical) {
	memset(&input_state, 0, sizeof(input_state));
	color_active = ws_system_set_mode(WS_MODE_COLOR);
	wsx_console_init_default(&wse_screen1);
	ws_display_set_screen_addresses(&wse_screen1, &wse_screen2);
	stdout = &fixed_console_file;
	memset(WS_TILE_MEM(0), 0, sizeof(ws_display_tile_t));
	memcpy(WS_TILE_MEM(RF_ICON_BASE), icon_tiles, sizeof(icon_tiles));
	if (color_active) {
		uint16_t ws_iram *ui = WS_SCREEN_COLOR_MEM(0);
		ui[0] = WS_RGB(15, 15, 14);
		ui[1] = WS_RGB(1, 1, 1);
		ui[2] = WS_RGB(1, 1, 1);
		ui[3] = WS_RGB(1, 1, 1);
	}
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE);
	rf_set_orientation(vertical);
	ws_int_set_default_handler_vblank();
	ws_int_enable(WS_INT_ENABLE_VBLANK);
	ia16_enable_irq();
	init_sound();
	rf_clear();
}

void rf_set_orientation(bool vertical) {
	ws_display_set_icons(vertical ? WS_LCD_ICON_ORIENT_V : WS_LCD_ICON_ORIENT_H);
}

void rf_frame(void) {
	uint16_t previous = input_state.held;

	ia16_halt();
	input_state.held = ws_keypad_scan();
	input_state.pressed = input_state.held & (uint16_t)~previous;
	input_state.released = previous & (uint16_t)~input_state.held;
	++frame_counter;

	if (tone_frames > 0) {
		--tone_frames;
		if (tone_frames == 0) {
			rf_sound_off();
		}
	}
}

void rf_clear(void) {
	text_x = 0;
	text_y = 0;
	playfield_mode = false;
	ws_screen_fill_tiles(&wse_screen1, WS_SCREEN_ATTR_TILE(0x1A0), 0, 0, 32, 32);
	if (active_art_map) {
		ws_screen_put_tiles(&wse_screen1, active_art_map, active_art_x, active_art_y,
			active_art_width, active_art_height);
	}
	ws_display_scroll_screen1_to(0, 0);
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE);
}

void rf_art_load(const uint8_t __far *tiles, uint16_t tile_bytes,
	const uint16_t __far *tilemap, uint8_t width, uint8_t height,
	uint8_t screen_x, uint8_t screen_y, const uint16_t __far *palette) {
	memcpy(WS_TILE_MEM(1), tiles, tile_bytes);
	active_art_map = tilemap;
	active_art_width = width;
	active_art_height = height;
	active_art_x = screen_x;
	active_art_y = screen_y;
	if (color_active) {
		uint8_t i;
		uint16_t ws_iram *art = WS_SCREEN_COLOR_MEM(1);
		uint16_t ws_iram *accent_1 = WS_SCREEN_COLOR_MEM(2);
		uint16_t ws_iram *accent_2 = WS_SCREEN_COLOR_MEM(3);
		for (i = 0; i < 4; ++i) art[i] = palette[i];
		accent_1[0] = palette[0];
		accent_2[0] = palette[0];
		for (i = 1; i < 4; ++i) {
			accent_1[i] = palette[2];
			accent_2[i] = palette[3];
		}
	} else {
		outportw(WS_SCR_PAL_1_PORT, WS_DISPLAY_MONO_PALETTE(0, 7, 4, 2));
	}
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE);
}

static uint16_t gfx_attr(uint16_t tile) {
	return WS_SCREEN_ATTR_TILE(tile) | WS_SCREEN_ATTR_PALETTE(0);
}

static void gfx_palette(const uint16_t __far *palette) {
	if (color_active) {
		memcpy(WS_SCREEN_COLOR_MEM(0), palette, 4 * sizeof(uint16_t));
	} else {
		outportw(WS_SCR_PAL_0_PORT, WS_DISPLAY_MONO_PALETTE(0, 7, 4, 2));
	}
}

void rf_gfx_show_intro(const uint8_t __far *tiles, uint16_t tile_bytes,
	const uint16_t __far *tilemap, const uint16_t __far *palette) {
	ws_display_set_control(0);
	memset(WS_TILE_MEM(0), 0, sizeof(ws_display_tile_t));
	memcpy(WS_TILE_MEM(1), tiles, tile_bytes);
	gfx_palette(palette);
	ws_screen_put_tiles(&wse_screen1, tilemap, 0, 0, 28, 18);
	ws_display_scroll_screen1_to(0, 0);
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE);
}

void rf_gfx_load(const uint8_t __far *tiles, uint16_t tile_bytes,
	const uint16_t __far *palette, uint16_t background_tile) {
	ws_display_set_control(0);
	memcpy(WS_TILE_MEM(0), tiles, tile_bytes);
	gfx_palette(palette);
	ws_screen_fill_tiles(&wse_screen1, gfx_attr(background_tile), 0, 0, 32, 32);
	ws_display_scroll_screen1_to(0, 0);
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE);
}

void rf_gfx_fill(uint16_t tile, uint8_t x, uint8_t y, uint8_t width,
	uint8_t height) {
	ws_screen_fill_tiles(&wse_screen1, gfx_attr(tile), x, y, width, height);
}

void rf_gfx_put_tile(uint8_t x, uint8_t y, uint16_t tile) {
	wse_screen1.row[y].cell[x] = gfx_attr(tile);
}

void rf_gfx_put_image(uint8_t x, uint8_t y, const uint16_t __far *tiles,
	uint8_t width, uint8_t height) {
	uint8_t image_y;
	uint8_t image_x;
	for (image_y = 0; image_y < height; ++image_y) {
		for (image_x = 0; image_x < width; ++image_x) {
			rf_gfx_put_tile((uint8_t)(x + image_x), (uint8_t)(y + image_y),
				tiles[(uint16_t)image_y * width + image_x]);
		}
	}
}

void rf_playfield_begin(void) {
	playfield_mode = true;
}

void rf_playfield_end(void) {
	playfield_mode = false;
}

void rf_header(const char __far *title, const char __far *subtitle) {
	printf("* ");
	printf(title);
	putchar('\n');
	printf(subtitle);
	putchar('\n');
	printf(rule);
	putchar('\n');
}

void rf_footer(const char __far *help) {
	printf(rule);
	putchar('\n');
	printf(help);
}

const rf_input_t *rf_input(void) {
	return &input_state;
}

int8_t rf_dx(uint16_t keys) {
	int8_t value = 0;
	if (keys & (WS_KEY_X4 | WS_KEY_Y1)) --value;
	if (keys & (WS_KEY_X2 | WS_KEY_Y3)) ++value;
	return value;
}

int8_t rf_dy(uint16_t keys) {
	int8_t value = 0;
	if (keys & (WS_KEY_X1 | WS_KEY_Y2)) --value;
	if (keys & (WS_KEY_X3 | WS_KEY_Y4)) ++value;
	return value;
}

bool rf_pressed_any_direction(void) {
	return rf_dx(input_state.pressed) != 0 || rf_dy(input_state.pressed) != 0;
}

uint16_t rf_frame_count(void) {
	return frame_counter;
}

uint16_t rf_random(void) {
	random_state ^= random_state << 7;
	random_state ^= random_state >> 9;
	random_state ^= random_state << 8;
	return random_state;
}

uint8_t rf_clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

void rf_print_bar(uint8_t value, uint8_t maximum, uint8_t width) {
	uint8_t filled = maximum ? (uint8_t)(((uint16_t)value * width) / maximum) : 0;
	uint8_t i;
	putchar('[');
	for (i = 0; i < width; ++i) {
		putchar(i < filled ? '#' : '.');
	}
	putchar(']');
}

void rf_tone(uint16_t hz, uint8_t volume) {
	outportw(WS_SOUND_FREQ_CH1_PORT, WS_SOUND_WAVE_HZ_TO_FREQ(hz, 32));
	outportb(WS_SOUND_VOL_CH1_PORT, (uint8_t)((volume << 4) | volume));
	outportb(WS_SOUND_CH_CTRL_PORT, WS_SOUND_CH_CTRL_CH1_ENABLE);
}

void rf_sound_off(void) {
	outportb(WS_SOUND_CH_CTRL_PORT, 0);
}

void rf_beep(uint16_t hz, uint8_t duration_frames) {
	rf_tone(hz, 6);
	tone_frames = duration_frames;
}
