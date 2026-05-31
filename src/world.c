#include <stdio.h>
#include <string.h>
#include "world.h"

// Load map from text file; rows padded with spaces if short
void world_load(World *w, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        // Fallback: blank room with border walls
        for (int y = 0; y < WORLD_H; y++) {
            for (int x = 0; x < WORLD_W; x++) {
                if (y == 0 || y == WORLD_H-1 || x == 0 || x == WORLD_W-1)
                    w->tiles[y][x] = '#';
                else
                    w->tiles[y][x] = '.';
            }
            w->tiles[y][WORLD_W] = '\0';
        }
        return;
    }
    for (int y = 0; y < WORLD_H; y++) {
        char line[WORLD_W + 4];
        if (fgets(line, sizeof(line), f)) {
            int len = (int)strlen(line);
            if (len > 0 && line[len-1] == '\n') len--;
            for (int x = 0; x < WORLD_W; x++) {
                w->tiles[y][x] = (x < len) ? line[x] : ' ';
            }
        } else {
            memset(w->tiles[y], ' ', WORLD_W);
        }
        w->tiles[y][WORLD_W] = '\0';
    }
    fclose(f);
}

bool world_is_wall(const World *w, int x, int y)
{
    if (x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return true;
    char t = w->tiles[y][x];
    return (t == '#');
}

bool world_is_food(const World *w, int x, int y)
{
    if (x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return false;
    return (w->tiles[y][x] == 'o');
}

char world_tile(const World *w, int x, int y)
{
    if (x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return '#';
    return w->tiles[y][x];
}
