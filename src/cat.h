#pragma once
#include <stdbool.h>

// Ear positions (predict interaction outcome per Deputte 2021)
typedef enum { EAR_FORWARD, EAR_SIDEWAYS, EAR_BACK, EAR_FLAT } EarPos;
// Tail positions (tail-up is human-directed; mostly down in cat-cat per Deputte 2021)
typedef enum { TAIL_DOWN, TAIL_UP, TAIL_TUCKED, TAIL_LASHING, TAIL_PUFFED } TailPos;

// Vocalization types (Tavernier et al. 2020, Table 1)
typedef enum {
    VOC_NONE, VOC_PURR, VOC_MEOW, VOC_HISS, VOC_GROWL,
    VOC_CHIRP, VOC_CHATTER, VOC_YOWL, VOC_SNARL,
    VOC_SPIT, VOC_MOAN, VOC_SQUEAK
} VocalType;

// Behavior IDs mapped to F-factors
typedef enum {
    BEH_IDLE,
    BEH_SLEEP,          // F4 Non-interacting
    BEH_GROOM_SELF,     // F4
    BEH_STALK,          // F3 Chasing
    BEH_WIGGLE,         // F3 pre-pounce
    BEH_POUNCE,         // F1 Wrestling
    BEH_BUNNY_KICK,     // F1
    BEH_CHASE,          // F3
    BEH_FLEE,           // F3
    BEH_WRESTLE,        // F1
    BEH_ALLOGROOM,      // F6 Prolonged interactivity
    BEH_VOCAL_CHIRP,    // F2 Vocalising
    BEH_VOCAL_HISS,     // F2
    BEH_VOCAL_GROWL,    // F2
    BEH_VOCAL_YOWL,     // F2
    BEH_COUNT
} Behavior;

#define MEMORY_SIZE 20

typedef struct {
    int   tick;      // global_tick at time of recording
    int   behavior;
    float valence;   // -1.0 bad, +1.0 good
} MemEntry;

typedef struct {
    // --- Identity ---
    char name[32];
    int  x, y;      // grid position
    int  dx, dy;    // last movement direction (-1, 0, or 1 each axis)

    // --- Physiology ---
    float energy;   // 0-100; -0.05/tick awake, +0.3/tick asleep
    float hunger;   // 0-100; +0.01/tick always
    float stress;   // 0-100
    float arousal;  // 0-100; passively decays toward baseline

    // --- Panksepp affect system ---
    float seeking;  // passively accumulates; decays after satisfaction
    float play;     // passively decays toward baseline
    float fear;
    float rage;
    float care;

    // --- Personality (0-1, fixed at init) ---
    float boldness;
    float sociability;
    float aggressiveness;
    float curiosity;
    float neuroticism;

    // --- Social ---
    float dominance;  // relative to other cat, -1 to +1

    // --- Body language ---
    EarPos  ears;
    TailPos tail;

    // --- Current behavior ---
    Behavior behavior;
    Behavior prev_behavior;   // previous tick's behavior for ticks-reset detection
    int      behavior_ticks;  // ticks spent in current behavior
    int      sleep_remaining; // ticks left in sleep bout
    bool     sleeping;

    // --- Vocalization ---
    VocalType vocal;
    int       vocal_ticks;

    // --- Memory ring buffer ---
    MemEntry memory[MEMORY_SIZE];
    int      mem_idx;

    // --- Active time tracking ---
    int active_ticks;
} Cat;

// Forward declare World for perception
typedef struct World World;

// cat.c
void cat_init(Cat *c, const char *name, int x, int y,
              float boldness, float sociability,
              float aggressiveness, float curiosity,
              float neuroticism, float dominance);
void update_physiology(Cat *c, float dt, int global_tick);
void perceive(Cat *self, Cat *other, World *w);
void vocalize(Cat *c, VocalType v);
void memory_record(Cat *c, int behavior, float valence, int tick);
int  count_positive_memories(Cat *c);
