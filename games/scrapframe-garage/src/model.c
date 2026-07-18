#include <string.h>

#include "model.h"

void scrapframe_reset(scrapframe_state_t *state) {
	memset(state, 0, sizeof(*state));
}

void scrapframe_step(scrapframe_state_t *state,
	const scrapframe_input_t *input, scrapframe_event_t *event) {
	static const uint8_t correct[3] = {0, 1, 2};
	memset(event, 0, sizeof(*event));
	if (state->phase == 0) {
		if (input->selection_direction) {
			state->selected = (uint8_t)((state->selected +
				(input->selection_direction > 0 ? 1 : 2)) % 3);
			event->tone_hz = input->selection_direction > 0 ? 110 : 92;
			event->tone_frames = 3;
			event->dirty = true;
		}
		if (input->confirm) {
			state->last_ok = state->selected == correct[state->job];
			if (state->last_ok) {
				++state->score;
				event->tone_hz = 660;
			} else {
				event->tone_hz = 130;
			}
			event->tone_frames = 10;
			state->phase = 1;
			event->dirty = true;
		}
	} else if (state->phase == 1 && input->confirm) {
		if (!state->last_ok) {
			/* A rejected part returns to the same bench for a readable retry. */
			state->phase = 0;
			state->selected = 0;
		} else if (++state->job >= 3) {
			state->phase = 2;
			event->tone_hz = 880;
			event->tone_frames = 18;
		}
		else {
			state->phase = 0;
			state->selected = 0;
		}
		event->dirty = true;
	} else if (state->phase == 2 && input->confirm) {
		scrapframe_reset(state);
		event->reset_session = true;
		event->dirty = true;
	}
}
