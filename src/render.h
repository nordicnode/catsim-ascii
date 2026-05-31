#pragma once
#include <ncurses.h>
#include "cat.h"
#include "world.h"

void render_init(void);
void render_world(const World *w);
void render_cats(const Cat *a, const Cat *b);
void render_statusbar(const Cat *a, const Cat *b, int sim_time_sec);
void render_flush(void);
void render_cleanup(void);

const char *vocal_name(VocalType v);
const char *behavior_name(Behavior b);
