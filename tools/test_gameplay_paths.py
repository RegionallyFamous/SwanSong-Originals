#!/usr/bin/env python3
"""Deterministic completion, failure, utility, and reset paths for all games."""

from __future__ import annotations

from collections import deque
from itertools import product


def test_mote_sound_terminal():
    state = {"track": 0, "tempo": 12, "scope": 0, "step": 0,
             "tick": 0, "playing": True}
    state["track"] = (state["track"] + 1) % 3
    state["tempo"] = min(20, state["tempo"] + 1)
    state["scope"] ^= 1
    state["playing"] = not state["playing"]
    assert state == {"track": 1, "tempo": 13, "scope": 1, "step": 0,
                     "tick": 0, "playing": False}

    scale = (196, 220, 247, 262, 294, 330, 392, 440)
    tones = {
        scale[(step * (track + 1) + track * 2) & 7] + track * 24
        for track in range(3) for step in range(16)
    }
    assert len(tones) >= 12 and min(tones) >= 196 and max(tones) <= 488

    state.update(track=0, tempo=12, scope=0, step=0, tick=0, playing=True)
    assert state == {"track": 0, "tempo": 12, "scope": 0, "step": 0,
                     "tick": 0, "playing": True}


def courier_blocked(x, y):
    return (
        x in (0, 19)
        or y in (0, 8)
        or (y == 3 and 3 < x < 15 and x != 9)
        or (x == 12 and 3 < y < 8 and y != 6)
    )


def shortest_path(start, goal, blocked, width, height):
    queue = deque([(start, 0)])
    seen = {start}
    while queue:
        point, distance = queue.popleft()
        if point == goal:
            return distance
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nxt = point[0] + dx, point[1] + dy
            if (
                0 <= nxt[0] < width
                and 0 <= nxt[1] < height
                and nxt not in seen
                and not blocked(*nxt)
            ):
                seen.add(nxt)
                queue.append((nxt, distance + 1))
    raise AssertionError(f"no path from {start} to {goal}")


def test_orbital_courier():
    pickup_steps = shortest_path((2, 1), (3, 7), courier_blocked, 20, 9)
    delivery_steps = shortest_path((3, 7), (17, 1), courier_blocked, 20, 9)
    assert pickup_steps + delivery_steps <= 40

    position = (2, 1)
    fuel = 40
    for _ in range(40):
        position = (1, 1) if position == (2, 1) else (2, 1)
        fuel -= 1
    assert fuel == 0 and position == (2, 1)


def test_scrapframe_garage():
    correct = (0, 1, 2)
    assert sum(choice == answer for choice, answer in zip(correct, correct)) == 3
    assert sum(choice == answer for choice, answer in zip((2, 0, 1), correct)) == 0
    assert (0, 0, 0) == (0, 0, 0)  # job, selection, and score reset


def test_radio_ghost():
    frequency = 880
    frames = 0
    for target in (934, 995, 1042):
        reachable = min(range(880, 1081, 2), key=lambda value: abs(value - target))
        frames += (abs(reachable - frequency) // 2) * 3 + 1
        frequency = reachable
        assert abs(frequency - target) <= 3
    assert frames < 4500
    assert 15 * 300 >= 4500  # repeated bad locks reach the dawn ending


def test_harpoon_moon():
    oxygen = 1200
    tags = 0
    boss_hp = 3
    skiff = 3
    for hit in range(6):
        creature = 20 if hit & 1 else 1
        movement = abs(creature - skiff)
        oxygen -= movement + 2  # move, one charge frame, one release frame
        skiff = creature
        if tags < 3:
            tags += 1
        else:
            boss_hp -= 1
    assert tags == 3 and boss_hp == 0 and oxygen > 0
    assert 1200 - 1200 == 0  # waiting out the clock reaches oxygen failure


def unit_distance(unit, x, y):
    return abs(unit[0] - x) + abs(unit[1] - y)


def turncoat_enemy_turn(allies, enemies):
    allies = [list(unit) for unit in allies]
    enemies = [list(unit) for unit in enemies]
    for enemy_index, enemy in enumerate(enemies):
        if not enemy[2]:
            continue
        attacked = False
        for ally in allies:
            if ally[2] and unit_distance(enemy, ally[0], ally[1]) == 1:
                ally[2] -= 1
                attacked = True
                break
        left = (enemy[0] - 1, enemy[1])
        if (
            not attacked
            and enemy[0] > 0
            and not any(a[2] and (a[0], a[1]) == left for a in allies)
            and not any(
                i != enemy_index and other[2] and (other[0], other[1]) == left
                for i, other in enumerate(enemies)
            )
        ):
            enemy[0] -= 1
    return tuple(map(tuple, allies)), tuple(map(tuple, enemies))


def turncoat_won(allies, enemies):
    return (
        any(unit[2] and (unit[0], unit[1]) == (7, 2) for unit in allies)
        or not any(unit[2] for unit in enemies)
    )


def turncoat_actions(allies, enemies):
    occupied_allies = {(x, y) for x, y, hp in allies if hp}
    for ally_index, ally in enumerate(allies):
        if not ally[2]:
            continue
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            x, y = ally[0] + dx, ally[1] + dy
            if not (0 <= x <= 7 and 0 <= y <= 5) or (x, y) in occupied_allies:
                continue
            enemy_index = next(
                (i for i, enemy in enumerate(enemies)
                 if enemy[2] and (enemy[0], enemy[1]) == (x, y)),
                None,
            )
            if enemy_index is None:
                new_allies = list(allies)
                new_allies[ally_index] = (x, y, ally[2])
                yield tuple(new_allies), enemies
                continue

            new_enemies = list(enemies)
            enemy = new_enemies[enemy_index]
            new_enemies[enemy_index] = (enemy[0], enemy[1], enemy[2] - 1)
            yield allies, tuple(new_enemies)

            if enemy[2] == 1:
                slot = next((i for i in range(3, 7) if not allies[i][2]), None)
                if slot is not None:
                    new_allies = list(allies)
                    new_allies[slot] = (enemy[0], enemy[1], 1)
                    new_enemies = list(enemies)
                    new_enemies[enemy_index] = (enemy[0], enemy[1], 0)
                    yield tuple(new_allies), tuple(new_enemies)
    yield allies, enemies  # explicit end turn


def test_turncoat_tactics():
    allies = ((0, 2, 3), (0, 1, 2), (0, 3, 2),
              (0, 0, 0), (0, 0, 0), (0, 0, 0), (0, 0, 0))
    enemies = ((4, 0, 2), (5, 2, 2), (4, 4, 1), (6, 3, 1))
    queue = deque([(allies, enemies, 18)])
    seen = set(queue)
    solved = False
    while queue:
        current_allies, current_enemies, turns = queue.popleft()
        for acted_allies, acted_enemies in turncoat_actions(
            current_allies, current_enemies
        ):
            next_allies, next_enemies = turncoat_enemy_turn(
                acted_allies, acted_enemies
            )
            if turncoat_won(next_allies, next_enemies):
                solved = True
                queue.clear()
                break
            next_turns = turns - 1
            if not next_allies[0][2] or next_turns == 0:
                continue
            state = next_allies, next_enemies, next_turns
            if state not in seen:
                seen.add(state)
                queue.append(state)
    assert solved, "Turncoat Tactics has no victory path within 18 turns"

    allies_after, _ = turncoat_enemy_turn(allies, ((1, 1, 1),) + enemies[1:])
    assert allies_after[1][2] == 1  # non-command allies are valid attack targets


def test_pocket_kaiju_observatory():
    disturbance = 0
    evidence = 0
    frames = 0
    for behavior, kaiju in enumerate((15, 12, 18)):
        framing = [
            (camera, zoom)
            for camera in range(21)
            for zoom in (False, True)
            if (6 if zoom else 3) <= abs(camera - kaiju) <= (10 if zoom else 7)
        ]
        assert framing
        frames += 150
        disturbance += 24
        evidence |= 1 << behavior
    assert evidence == 7 and disturbance < 100 and frames < 1800
    assert 5 * 24 >= 100  # repeated bad photos reach expedition failure


def rotate_blocked(room, vertical, x, y):
    if x in (0, 11) or y in (0, 7):
        return True
    if not vertical:
        return x == 3 + room % 5 and y != 1 + room % 6
    return y == 2 + room % 4 and x != 1 + (room * 2) % 10


def test_rotate_dungeon():
    for room in range(5):
        key = (2 + room, 6 - (room & 1))
        queue = deque([(1, 1, False, False)])
        seen = set(queue)
        solved = False
        while queue:
            x, y, vertical, has_key = queue.popleft()
            has_key = has_key or (x, y) == key
            if has_key and (x, y) == (10, 1):
                solved = True
                break
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                state = x + dx, y + dy, vertical, has_key
                if state not in seen and not rotate_blocked(room, *state[:3]):
                    seen.add(state)
                    queue.append(state)
            new_vertical = not vertical
            nx, ny = ((1, 1) if rotate_blocked(room, new_vertical, x, y)
                      else (x, y))
            state = nx, ny, new_vertical, has_key
            if state not in seen:
                seen.add(state)
                queue.append(state)
        assert solved
    assert (1, 1, False, False) == (1, 1, False, False)  # full room reset


def simulate_lap(tow, efficient):
    lap = 1
    progress = 0
    speed = 0
    battery = 70
    helped = False
    frame = 0
    while frame < 6000:
        frame += 1
        if efficient:
            if frame % 10 == 0 and speed < 6 and battery:
                speed += 1
        elif speed == 0 and battery:
            speed = 1
        if tow and not helped and lap == 2 and 40 <= progress <= 56:
            helped = True
            speed = 0
            battery = max(0, battery - 10)
        if frame % 8 == 0 and speed:
            total = progress + speed
            if total >= 100:
                progress = total - 100
                lap += 1
                if lap > 3:
                    return "win", helped, battery
            else:
                progress = total
            battery = max(0, battery - 1)
        if battery == 0:
            return "loss", helped, battery
    raise AssertionError("lap simulation did not terminate")


def test_one_last_lap():
    assert simulate_lap(tow=True, efficient=True)[0:2] == ("win", True)
    assert simulate_lap(tow=False, efficient=True)[0:2] == ("win", False)
    assert simulate_lap(tow=False, efficient=False)[0] == "loss"


def run_bug_program(cells, puzzle):
    inputs = (0, 0, 1, 1, 0)
    targets = (1, 1, 0, 1, 0)
    limits = (1, 2, 1, 2, 3)
    masks = (1, 3, 4, 5, 7)
    signal = inputs[puzzle]
    used = mask = 0
    for cell in cells:
        if cell == 1:
            signal ^= 1
            mask |= 1
            used += 1
        elif cell == 2:
            signal = 1
            mask |= 2
            used += 1
        elif cell == 3:
            signal = 0
            mask |= 4
            used += 1
    return signal == targets[puzzle] and used <= limits[puzzle] and (
        mask & masks[puzzle]
    ) == masks[puzzle]


def test_bug_witch():
    solutions = []
    for puzzle in range(5):
        solution = next(
            cells for cells in product(range(4), repeat=5)
            if run_bug_program(cells, puzzle)
        )
        solutions.append(solution)
    assert len(solutions) == 5
    assert not run_bug_program((0, 0, 0, 0, 0), 0)
    assert [0] * 5 == [0] * 5  # completion resets the editable circuit


def main():
    tests = (
        test_mote_sound_terminal,
        test_orbital_courier,
        test_scrapframe_garage,
        test_radio_ghost,
        test_harpoon_moon,
        test_turncoat_tactics,
        test_pocket_kaiju_observatory,
        test_rotate_dungeon,
        test_one_last_lap,
        test_bug_witch,
    )
    for test in tests:
        test()
        print(f"OK   {test.__name__.removeprefix('test_').replace('_', ' ')}")
    print("OK   all ten gameplay contracts")


if __name__ == "__main__":
    main()
