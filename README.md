# CatSim ASCII

Terminal ASCII simulation of two cats (Milo & Luna) with ethology-grounded behavioral AI.
Built in C17 using ncurses. Target platform: **CachyOS Linux** (Arch-based).

## Install & Build (CachyOS)

```bash
# CachyOS is Arch-based with optimized repos (5-20% faster builds)
sudo pacman -Syu
sudo pacman -S base-devel ncurses git

# Verify toolchain
gcc --version

# Test ncurses
echo '#include <ncurses.h>
int main(){initscr();endwin();return 0;}' > test.c
gcc test.c -lncursesw -o test && ./test && echo OK
rm test.c test

# Clone and build
git clone https://github.com/nordicnode/catsim-ascii
cd catsim-ascii
make
./catsim
```

Press `q` to quit.

## File Structure

```
catsim-ascii/
├── Makefile
├── README.md
├── src/
│   ├── main.c        – 10 Hz ncurses loop, clock_nanosleep timer
│   ├── cat.h / cat.c – Cat struct, physiology, perception, memory
│   ├── behavior.h / behavior.c – 14 ethogram behaviors (F1–F6)
│   ├── fcm.h / fcm.c – Fuzzy Cognitive Map (6 factor nodes)
│   ├── world.h / world.c – Map loader, wall/food detection
│   └── render.h / render.c – ncurses ASCII drawing, status bar
└── data/
    ├── personalities.json – Milo & Luna traits
    └── map_default.txt    – 80×22 house map
```

## Behavioral Science Basis

| System | Source |
|--------|--------|
| 6 interaction factors (F1–F6) | Ramos et al. 2017 (105 dyad study) |
| Outcome distribution 56/15/29% | Ramos et al. 2017 |
| Ear position → outcome prediction | Deputte 2021 |
| Allogrooming stats (91.6% unidirectional) | Alberts et al. 2002 |
| Sleep budget 12–16h/day | Montoya et al. 2023 |
| Prey sequence (stalk→wiggle→pounce) | Pellis & Pellis 2009 |
| Vocal repertoire (10+ types) | Tavernier et al. 2020 |

## Simulation Mechanics

- **10 Hz tick rate** using `clock_nanosleep` (CLOCK_MONOTONIC)
- **FCM behavior selection**: `argmax(F1–F6)` above threshold 0.3 — no randomness
- **Circadian rhythm**: every 600 ticks, cats with energy < 40 sleep 3–5 real minutes
- **Memory ring buffer**: 20 entries, drives allogrooming unlock after 3+ positive interactions
- **ASCII legend**: `c` sleeping · `&` alert/active · `@` pouncing · `*` bunny-kick · `>` ears-back marker · `~` tail lashing
