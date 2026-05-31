#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "cat.h"
#include "world.h"

void cat_init(Cat *c, const char *name, int x, int y,
              float boldness, float sociability,
              float aggressiveness, float curiosity,
              float neuroticism, float dominance)
{
    memset(c, 0, sizeof(*c));
    strncpy(c->name, name, 31);
    c->name[31] = '\0';  // guarantee null termination
    c->x = x; c->y = y;
    c->dx = 0; c->dy = 0;
    c->energy    = 80.0f;
    c->hunger    = 20.0f;
    c->stress    = 10.0f;
    c->arousal   = 30.0f;
    c->seeking   = 0.4f;
    c->play      = 0.4f;
    c->fear      = 0.1f;
    c->rage      = 0.05f;
    c->care      = 0.3f;
    c->boldness        = boldness;
    c->sociability     = sociability;
    c->aggressiveness  = aggressiveness;
    c->curiosity       = curiosity;
    c->neuroticism     = neuroticism;
    c->dominance       = dominance;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_DOWN;
    c->behavior = BEH_IDLE;
    c->prev_behavior = BEH_IDLE;
    c->sleeping = false;
    c->sleep_remaining = 0;
    c->active_ticks = 0;
    c->vocal = VOC_NONE;
    c->vocal_ticks = 0;
    c->mem_idx = 0;
}

void update_physiology(Cat *c, float dt, int global_tick)
{
    c->hunger += 0.01f * dt;
    if (c->hunger > 100.0f) c->hunger = 100.0f;

    if (c->sleeping) {
        c->energy += 0.3f * dt;
        if (c->energy > 100.0f) c->energy = 100.0f;
        c->stress -= 0.05f * dt;
        if (c->stress < 0.0f) c->stress = 0.0f;
        c->sleep_remaining--;
        if (c->sleep_remaining <= 0) {
            c->sleeping = false;
            c->behavior = BEH_IDLE;
        }
        return;
    }

    c->energy -= 0.05f * dt;
    if (c->energy < 0.0f) c->energy = 0.0f;
    c->active_ticks++;

    // Force low-energy non-interaction
    if (c->energy < 20.0f) {
        c->behavior = BEH_IDLE;
        c->ears = EAR_SIDEWAYS;
    }

    // Circadian sleep trigger
    if ((global_tick % 600 == 0) && c->energy < 40.0f) {
        c->sleeping = true;
        c->behavior = BEH_SLEEP;
        c->sleep_remaining = 180 + rand() % 121;
    }

    // Passive accumulation: seeking drive builds while active (resets at satiation via pounce)
    if (c->active_ticks < 3600) {
        c->seeking += 0.0001f * dt;
        if (c->seeking > 1.0f) c->seeking = 1.0f;
    } else {
        // After prolonged wakefulness, seeking decays back toward baseline
        c->seeking -= 0.0001f * dt;
        if (c->seeking < 0.0f) c->seeking = 0.0f;
    }

    // Passive play decay: play drive returns to baseline over time
    c->play -= 0.002f * dt;
    if (c->play < 0.0f) c->play = 0.0f;

    // Passive arousal decay: excitement returns to resting baseline
    c->arousal -= 0.05f * dt;
    if (c->arousal < 0.0f) c->arousal = 0.0f;

    // Accumulate care drive over time to unlock social requirements
    c->care += 0.002f * dt;
    if (c->care > 1.0f) c->care = 1.0f;

    // Vocal timeout
    if (c->vocal_ticks > 0) {
        c->vocal_ticks--;
        if (c->vocal_ticks == 0) c->vocal = VOC_NONE;
    }
}

static bool has_los(Cat *self, Cat *other, World *w)
{
    int x0 = self->x, y0 = self->y;
    int x1 = other->x, y1 = other->y;
    int dx = abs(x1-x0), dy = abs(y1-y0);
    int sx = (x0<x1)?1:-1, sy = (y0<y1)?1:-1;
    int err = dx - dy;
    while (x0 != x1 || y0 != y1) {
        if (world_is_wall(w, x0, y0)) return false;
        int e2 = 2*err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
    return true;
}

void perceive(Cat *self, Cat *other, World *w)
{
    float ddx = (float)(other->x - self->x);
    float ddy = (float)(other->y - self->y);
    float dist = sqrtf(ddx*ddx + ddy*ddy);

    if (dist < 8.0f && has_los(self, other, w)) {
        if (other->ears == EAR_FORWARD && other->tail != TAIL_PUFFED) {
            self->fear -= 0.1f;
        }
        if (other->ears == EAR_FLAT || other->ears == EAR_BACK) {
            self->fear += 0.15f;
            self->rage += 0.05f;
        }
        if (self->ears == EAR_FORWARD && other->ears == EAR_FORWARD) {
            self->play  += 0.2f;
            self->fear  -= 0.2f;
        }
        self->arousal += (8.0f - dist) * 0.01f;
    }
    if (self->fear   < 0.0f) self->fear   = 0.0f;
    if (self->fear   > 1.0f) self->fear   = 1.0f;
    if (self->play   < 0.0f) self->play   = 0.0f;
    if (self->play   > 1.0f) self->play   = 1.0f;
    if (self->rage   < 0.0f) self->rage   = 0.0f;
    if (self->rage   > 1.0f) self->rage   = 1.0f;
    if (self->arousal < 0.0f) self->arousal = 0.0f;
    if (self->arousal > 100.0f) self->arousal = 100.0f;
}

void vocalize(Cat *c, VocalType v)
{
    c->vocal = v;
    c->vocal_ticks = 3;
}

void memory_record(Cat *c, int behavior, float valence, int tick)
{
    // Avoid flooding ring buffer with identical sequential events at 10Hz
    int prev_idx = (c->mem_idx - 1 + MEMORY_SIZE) % MEMORY_SIZE;
    if (c->memory[prev_idx].behavior == behavior
     && c->memory[prev_idx].valence  == valence) {
        return; // Skip duplicating active persistent loops
    }

    c->memory[c->mem_idx].tick     = tick;
    c->memory[c->mem_idx].behavior = behavior;
    c->memory[c->mem_idx].valence  = valence;
    c->mem_idx = (c->mem_idx + 1) % MEMORY_SIZE;
}

int count_positive_memories(Cat *c)
{
    int count = 0;
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (c->memory[i].valence > 0.0f) count++;
    }
    return count;
}
