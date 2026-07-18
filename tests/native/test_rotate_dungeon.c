#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "model.h"

#define ROUTE_NODE_CAPACITY 512

typedef struct {
	rotate_state_t state;
	int16_t parent;
	char action;
} route_node_t;

static void apply_action(rotate_state_t *state, char action, rotate_event_t *event) {
	rotate_input_t input = {0, 0, false, false, false};
	if (action == 'S') input.rotate = true;
	else if (action == 'U') input.dy = -1;
	else if (action == 'D') input.dy = 1;
	else if (action == 'L') input.dx = -1;
	else if (action == 'R') input.dx = 1;
	else assert(0 && "unknown route action");
	rotate_step(state, &input, event);
}

static bool same_search_state(const rotate_state_t *left,
	const rotate_state_t *right) {
	return left->room == right->room && left->x == right->x &&
		left->y == right->y && left->result == right->result &&
		left->vertical == right->vertical && left->key == right->key;
}

static void solve_room(const rotate_state_t *start, char *route) {
	static const char actions[] = {'S', 'U', 'D', 'L', 'R'};
	route_node_t nodes[ROUTE_NODE_CAPACITY];
	uint16_t head = 0;
	uint16_t tail = 1;
	uint16_t goal = UINT16_MAX;
	uint16_t i;

	nodes[0].state = *start;
	nodes[0].parent = -1;
	nodes[0].action = 0;
	while (head < tail && goal == UINT16_MAX) {
		uint8_t action_index;
		for (action_index = 0; action_index < sizeof(actions); ++action_index) {
			rotate_event_t event;
			rotate_state_t next = nodes[head].state;
			bool seen = false;
			apply_action(&next, actions[action_index], &event);
			if (next.room != start->room || next.result == ROTATE_RESULT_COMPLETE) {
				assert(tail < ROUTE_NODE_CAPACITY);
				nodes[tail] = (route_node_t){next, (int16_t)head,
					actions[action_index]};
				goal = tail++;
				break;
			}
			for (i = 0; i < tail; ++i) {
				if (same_search_state(&nodes[i].state, &next)) {
					seen = true;
					break;
				}
			}
			if (!seen) {
				assert(tail < ROUTE_NODE_CAPACITY);
				nodes[tail++] = (route_node_t){next, (int16_t)head,
					actions[action_index]};
			}
		}
		++head;
	}
	assert(goal != UINT16_MAX);
	{
		char reversed[64];
		uint8_t length = 0;
		while (nodes[goal].parent >= 0) {
			reversed[length++] = nodes[goal].action;
			goal = (uint16_t)nodes[goal].parent;
		}
		for (i = 0; i < length; ++i) route[i] = reversed[length - i - 1];
		route[length] = '\0';
	}
}

int main(void) {
	static const char *canonical_routes[] = {
		"SDDDDDRRSRRRRRRRRR",
		"DDDDRRUUURRURRRRR",
		"DDDDDRRRUUURRUURRRR",
		"DDDDRRRRURRUUURRR",
		"DDDDDRRRRRURRUUUURR",
	};
	rotate_state_t state;
	rotate_state_t initial;
	rotate_input_t input = {0, 0, false, false, false};
	rotate_event_t event;
	uint8_t room;

	rotate_reset(&state);
	initial = state;
	assert(state.x == 1 && state.y == 1 && !state.vertical);
	assert(rotate_blocked(0, false, 0, 1));
	input.rotate = true;
	rotate_step(&state, &input, &event);
	assert(state.vertical && event.orientation_changed && event.tone_hz == 520);
	input.rotate = false;
	input.reset_room = true;
	rotate_step(&state, &input, &event);
	assert(!state.vertical && state.x == 1 && state.y == 1);

	input.reset_room = false;
	for (room = 0; room < 5; ++room) {
		char route[64];
		uint8_t step;
		assert(state.room == room);
		solve_room(&state, route);
		assert(strcmp(route, canonical_routes[room]) == 0);
		for (step = 0; route[step]; ++step)
			apply_action(&state, route[step], &event);
		assert(state.room == (uint8_t)(room + 1));
		assert(event.tone_hz == (room == 4 ? 760 : 640));
	}
	assert(state.result == ROTATE_RESULT_COMPLETE && event.tone_frames == 12);
	input.replay = true;
	rotate_step(&state, &input, &event);
	assert(event.reset_session && memcmp(&state, &initial, sizeof(state)) == 0);
	return 0;
}
