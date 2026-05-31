#pragma once
#include <stdbool.h>

#define WORLD_W 80
#define WORLD_H 22

typedef struct World {
    char tiles[WORLD_H][WORLD_W + 1];  // +1 for null terminator per row
} World;

void world_load(World *w, const char *path);
bool world_is_wall(const World *w, int x, int y);
bool world_is_food(const World *w, int x, int y);
char world_tile(const World *w, int x, int y);
