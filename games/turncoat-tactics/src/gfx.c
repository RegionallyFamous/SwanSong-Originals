#include <swan/legacy.h>

#include "swan_game_runtime.h"
#include "gfx.h"
#include "gameplay_art.h"

static bool board_valid;
static turncoat_state_t rendered_state;

static const uint16_t __far *unit_for(uint8_t team, uint8_t kind) {
	if (team == 0) {
		if (kind == 0) return art_unit_0_0;
		if (kind == 1) return art_unit_0_1;
		return art_unit_0_2;
	}
	if (kind == 0) return art_unit_1_0;
	if (kind == 1) return art_unit_1_1;
	return art_unit_1_2;
}

static int8_t unit_at(const unit_t *units, uint8_t capacity, uint8_t x, uint8_t y) {
	uint8_t i;
	for (i = 0; i < capacity; ++i) {
		if (units[i].hp && units[i].x == (int8_t)x && units[i].y == (int8_t)y)
			return (int8_t)i;
	}
	return -1;
}

static bool in_selected_range(const turncoat_state_t *state, uint8_t x, uint8_t y) {
	const unit_t *selected = &state->allies[state->selected];
	uint8_t dx;
	uint8_t dy;
	if (!selected->hp) return false;
	dx = selected->x > (int8_t)x ? (uint8_t)(selected->x - (int8_t)x) :
		(uint8_t)((int8_t)x - selected->x);
	dy = selected->y > (int8_t)y ? (uint8_t)(selected->y - (int8_t)y) :
		(uint8_t)((int8_t)y - selected->y);
	return (uint8_t)(dx + dy) == 1;
}

static uint16_t cell_signature(const turncoat_state_t *state, uint8_t x, uint8_t y) {
	int8_t ai = unit_at(state->allies, ALLY_CAPACITY, x, y);
	int8_t ei = unit_at(state->enemies, ENEMY_CAPACITY, x, y);
	uint16_t signature = 0;
	if (x == 7 && y == 2) signature |= 1u;
	if (state->cursor_x == x && state->cursor_y == y) signature |= 2u;
	if (in_selected_range(state, x, y)) signature |= 4u;
	if (ai >= 0) {
		signature |= (uint16_t)((uint16_t)(ai + 1) << 3);
		signature |= (uint16_t)((uint16_t)state->allies[(uint8_t)ai].hp << 6);
		if ((uint8_t)ai == state->selected) signature |= 0x0100u;
	}
	if (ei >= 0) {
		signature |= (uint16_t)((uint16_t)(ei + 1) << 9);
		signature |= (uint16_t)((uint16_t)state->enemies[(uint8_t)ei].hp << 12);
	}
	return signature;
}

static void draw_cell(const turncoat_state_t *state, uint8_t x, uint8_t y) {
	uint8_t sx = (uint8_t)(2 + x * 3);
	uint8_t sy = (uint8_t)(3 + y * 2);
	int8_t ai = unit_at(state->allies, ALLY_CAPACITY, x, y);
	int8_t ei = unit_at(state->enemies, ENEMY_CAPACITY, x, y);
	uint8_t hp;

	rf_gfx_put_image(sx, sy, art_cell, 3, 2);
	if (x == 7 && y == 2) rf_gfx_put_tile((uint8_t)(sx + 1),
		(uint8_t)(sy + 1), art_beacon[0]);
	if (in_selected_range(state, x, y))
		rf_gfx_put_tile((uint8_t)(sx + 2), sy, art_pip_empty[0]);
	if (ai >= 0) {
		const unit_t *unit = &state->allies[(uint8_t)ai];
		for (hp = 0; hp < unit->hp; ++hp)
			rf_gfx_put_tile((uint8_t)(sx + hp), sy, art_pip_full[0]);
		rf_gfx_put_image((uint8_t)(sx + 1), (uint8_t)(sy + 1),
			unit_for(0, (uint8_t)ai % 3), 1, 1);
		if ((uint8_t)ai == state->selected)
			rf_gfx_put_tile((uint8_t)(sx + 2), (uint8_t)(sy + 1), art_cursor[0]);
	}
	if (ei >= 0) {
		const unit_t *unit = &state->enemies[(uint8_t)ei];
		for (hp = 0; hp < unit->hp; ++hp)
			rf_gfx_put_tile((uint8_t)(sx + hp), sy, art_pip_full[0]);
		rf_gfx_put_image((uint8_t)(sx + 1), (uint8_t)(sy + 1),
			unit_for(1, (uint8_t)ei % 3), 1, 1);
	}
	if (state->cursor_x == x && state->cursor_y == y)
		rf_gfx_put_tile(sx, (uint8_t)(sy + 1), art_cursor[0]);
}

void gfx_show_intro(void) {
	swan_game_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
		game_intro_map, game_palette);
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
	board_valid = false;
}

void gfx_render(const turncoat_state_t *state) {
	uint8_t y;
	uint8_t x;
	uint8_t i;
	const unit_t *allies = state->allies;

	if (!board_valid) rf_gfx_fill(art_hud_bg[0], 0, 0, 28, 18);
	for (i = 0; i < 9; ++i) {
		rf_gfx_put_tile((uint8_t)(1 + i), 0,
			state->turns > (uint8_t)(i * 2) ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (i = 0; i < 3; ++i) {
		rf_gfx_put_tile((uint8_t)(13 + i), 0,
			i < allies[0].hp ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (i = 0; i < 4; ++i) {
		rf_gfx_put_tile((uint8_t)(21 + i), 0,
			i < state->recruits ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (y = 0; y < 6; ++y) {
		for (x = 0; x < 8; ++x) {
			if (!board_valid || cell_signature(&rendered_state, x, y) !=
				cell_signature(state, x, y)) draw_cell(state, x, y);
		}
	}
	if (state->result) {
		rf_gfx_put_image(12, 7,
			state->result == TURNCOAT_RESULT_WIN ? art_result_win : art_result_loss,
			4, 4);
		rf_gfx_put_image(13, 11, art_loop, 2, 2);
	}
	rendered_state = *state;
	board_valid = true;
}
