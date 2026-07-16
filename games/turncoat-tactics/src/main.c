#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "rf_swan.h"
#include "gfx.h"

static unit_t allies[ALLY_CAPACITY];
static unit_t enemies[ENEMY_CAPACITY];

static uint8_t distance(const unit_t *unit, int8_t x, int8_t y) {
	uint8_t dx = unit->x > x ? (uint8_t)(unit->x - x) : (uint8_t)(x - unit->x);
	uint8_t dy = unit->y > y ? (uint8_t)(unit->y - y) : (uint8_t)(y - unit->y);
	return (uint8_t)(dx + dy);
}

static int8_t ally_at(int8_t x, int8_t y) {
	uint8_t i;
	for (i = 0; i < ALLY_CAPACITY; ++i) if (allies[i].hp && allies[i].x == x && allies[i].y == y) return (int8_t)i;
	return -1;
}

static int8_t enemy_at(int8_t x, int8_t y) {
	uint8_t i;
	for (i = 0; i < ENEMY_CAPACITY; ++i) if (enemies[i].hp && enemies[i].x == x && enemies[i].y == y) return (int8_t)i;
	return -1;
}

static uint8_t enemy_count(void) {
	uint8_t count = 0;
	uint8_t i;
	for (i = 0; i < ENEMY_CAPACITY; ++i) if (enemies[i].hp) ++count;
	return count;
}

static int8_t empty_recruit_slot(void) {
	uint8_t i;
	for (i = 3; i < ALLY_CAPACITY; ++i) if (!allies[i].hp) return (int8_t)i;
	return -1;
}

static uint8_t first_living_ally(void) {
	uint8_t i;
	for (i = 0; i < ALLY_CAPACITY; ++i) if (allies[i].hp) return i;
	return 0;
}

static bool beacon_secured(void) {
	uint8_t i;
	for (i = 0; i < ALLY_CAPACITY; ++i) {
		if (allies[i].hp && allies[i].x == 7 && allies[i].y == 2) return true;
	}
	return false;
}

static void reset_units(void) {
	memset(allies, 0, sizeof(allies));
	memset(enemies, 0, sizeof(enemies));
	allies[0] = (unit_t){0, 2, 3};
	allies[1] = (unit_t){0, 1, 2};
	allies[2] = (unit_t){0, 3, 2};
	enemies[0] = (unit_t){4, 0, 2};
	enemies[1] = (unit_t){5, 2, 2};
	enemies[2] = (unit_t){4, 4, 1};
	enemies[3] = (unit_t){6, 3, 1};
}

static void enemy_turn(void) {
	uint8_t i;
	for (i = 0; i < ENEMY_CAPACITY; ++i) {
		uint8_t ally;
		bool attacked = false;
		if (!enemies[i].hp) continue;
		for (ally = 0; ally < ALLY_CAPACITY; ++ally) {
			if (allies[ally].hp && distance(&enemies[i], allies[ally].x, allies[ally].y) == 1) {
				--allies[ally].hp;
				attacked = true;
				break;
			}
		}
		if (!attacked && enemies[i].x > 0 &&
			ally_at((int8_t)(enemies[i].x - 1), enemies[i].y) < 0 &&
			enemy_at((int8_t)(enemies[i].x - 1), enemies[i].y) < 0) {
			--enemies[i].x;
		}
	}
}

void main(void) {
	uint8_t cursor_x = 0;
	uint8_t cursor_y = 2;
	uint8_t selected = 0;
	uint8_t turns = 18;
	uint8_t recruits = 0;
	uint8_t result = 0;
	bool dirty = true;
	uint8_t intro;

	reset_units();
	rf_init(false);
	gfx_show_intro();
	for (intro = 0; intro < 36; ++intro) {
		rf_frame();
		if (rf_input()->pressed) break;
	}
	gfx_init();
	while (1) {
		const rf_input_t *input;
		bool acted = false;
		rf_frame();
		input = rf_input();

		if (result && (input->pressed & WS_KEY_A)) {
			reset_units(); cursor_x = 0; cursor_y = 2; selected = 0;
			turns = 18; recruits = 0; result = 0; dirty = true;
		}
		if (!result) {
			int8_t dx = rf_dx(input->pressed);
			int8_t dy = rf_dy(input->pressed);
			if (dx || dy) {
				cursor_x = rf_clamp_u8((int16_t)cursor_x + dx, 0, 7);
				cursor_y = rf_clamp_u8((int16_t)cursor_y + dy, 0, 5);
				dirty = true;
			}
			if (input->pressed & WS_KEY_A) {
				int8_t ai = ally_at((int8_t)cursor_x, (int8_t)cursor_y);
				int8_t ei = enemy_at((int8_t)cursor_x, (int8_t)cursor_y);
				if (ai >= 0) selected = (uint8_t)ai;
				else if (distance(&allies[selected], (int8_t)cursor_x, (int8_t)cursor_y) == 1) {
					if (ei >= 0) --enemies[(uint8_t)ei].hp;
					else { allies[selected].x = (int8_t)cursor_x; allies[selected].y = (int8_t)cursor_y; }
					acted = true;
				}
				dirty = true;
			}
			if (input->pressed & WS_KEY_B) {
				int8_t ei = enemy_at((int8_t)cursor_x, (int8_t)cursor_y);
				int8_t slot = empty_recruit_slot();
				if (ei >= 0 && enemies[(uint8_t)ei].hp == 1 &&
					slot >= 0 &&
					distance(&allies[selected], (int8_t)cursor_x, (int8_t)cursor_y) == 1) {
					allies[(uint8_t)slot] = (unit_t){enemies[(uint8_t)ei].x,
						enemies[(uint8_t)ei].y, 1};
					enemies[(uint8_t)ei].hp = 0; ++recruits; acted = true;
					rf_beep(580, 10);
				}
			}
			if (input->pressed & WS_KEY_START) acted = true;
			if (acted) {
				if (turns) --turns;
				enemy_turn();
				if (!allies[selected].hp) selected = first_living_ally();
				dirty = true;
			}
			if (beacon_secured() || enemy_count() == 0) result = 1;
			else if (allies[0].hp == 0 || turns == 0) result = 2;
		}
		if (dirty) {
			gfx_render(cursor_x, cursor_y, selected, turns, recruits, result,
				allies, enemies);
			dirty = false;
		}
	}
}
