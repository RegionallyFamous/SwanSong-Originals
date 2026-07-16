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
static ws_sound_wavetable_t sound_waves __attribute__((aligned(64)));

static const char __far rule[] = "==//====================//==";

static int fixed_console_put(uint8_t c, FILE *stream) {
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
	wse_screen1.row[text_y].cell[text_x] =
		WS_SCREEN_ATTR_TILE(0x1A0 + c - 0x20);
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
	wsx_console_init_default(&wse_screen1);
	stdout = &fixed_console_file;
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
	ws_screen_fill_tiles(&wse_screen1, WS_SCREEN_ATTR_TILE(0x1A0), 0, 0, 32, 32);
	ws_display_scroll_screen1_to(0, 0);
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE);
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
