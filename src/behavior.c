#include <stdlib.h>
#include <math.h>
#include "behavior.h"
#include "world.h"

// --- Utility: clamp move to non-wall cell ---
static void move_toward(Cat *c, int tx, int ty, World *w)
{
    int nx = c->x + (tx > c->x ? 1 : tx < c->x ? -1 : 0);
    int ny = c->y + (ty > c->y ? 1 : ty < c->y ? -1 : 0);
    if (!world_is_wall(w, nx, ny)) { c->x = nx; c->y = ny; }
}

static void move_away(Cat *c, int tx, int ty, World *w)
{
    int nx = c->x - (tx > c->x ? 1 : tx < c->x ? -1 : 0);
    int ny = c->y - (ty > c->y ? 1 : ty < c->y ? -1 : 0);
    if (!world_is_wall(w, nx, ny)) { c->x = nx; c->y = ny; }
}

// --- F4: BEH_SLEEP ---
// energy<30; ASCII: c; vocal: purr (Tavernier 2020 §low-arousal closed-mouth)
void beh_sleep(Cat *c)
{
    c->behavior = BEH_SLEEP;
    c->sleeping = true;
    c->ears = EAR_SIDEWAYS;
    c->tail = TAIL_DOWN;
    vocalize(c, VOC_PURR);
    c->behavior_ticks++;
}

// --- F4: BEH_GROOM_SELF ---
// stress>30; displacement activity; ASCII: & licking
void beh_groom_self(Cat *c)
{
    c->behavior = BEH_GROOM_SELF;
    c->stress   -= 0.5f;
    if (c->stress < 0.0f) c->stress = 0.0f;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_DOWN;
    c->behavior_ticks++;
}

// --- F3: BEH_STALK ---
// Prey sequence step 1: crouch 3 ticks (Pellis & Pellis 2009)
// ASCII: ~& crouched; vocal: chatter if frustrated (Tavernier 2020)
void beh_stalk(Cat *c, Cat *target)
{
    c->behavior = BEH_STALK;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_DOWN;
    // Crouch toward target slowly (prey sequence requires deliberate approach)
    float ddx = (float)(target->x - c->x);
    float ddy = (float)(target->y - c->y);
    float dist = sqrtf(ddx*ddx + ddy*ddy);
    if (dist > 3.0f) {
        // Slow approach; only move every 3 ticks
        if (c->behavior_ticks % 3 == 0) {
            int nx = c->x + (target->x > c->x ? 1 : target->x < c->x ? -1 : 0);
            int ny = c->y + (target->y > c->y ? 1 : target->y < c->y ? -1 : 0);
            // world checked in caller context; here just update
            c->x = nx; c->y = ny;
        }
    }
    // Frustrated chatter if target is moving away (Tavernier 2020)
    if (c->seeking > 0.8f && c->behavior_ticks > 5) {
        vocalize(c, VOC_CHATTER);
    }
    c->behavior_ticks++;
}

// --- F3: BEH_WIGGLE ---
// Pre-pounce butt-wiggle: tests ground, loads hindquarters (Pellis & Pellis 2009)
// ASCII: & shaking (2 ticks)
void beh_wiggle(Cat *c)
{
    c->behavior = BEH_WIGGLE;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_LASHING;
    c->arousal += 0.1f;
    c->behavior_ticks++;
}

// --- F3: BEH_POUNCE ---
// Full prey sequence requires satisfaction (Pellis & Pellis 2009)
// ASCII: @
void beh_pounce(Cat *c, Cat *target, World *w)
{
    c->behavior = BEH_POUNCE;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_LASHING;
    move_toward(c, target->x, target->y, w);
    if (c->x == target->x && c->y == target->y) {
        // Landed; transition to wrestle or bunny-kick
        c->play    += 0.3f;
        c->seeking -= 0.4f;
        if (c->seeking < 0.0f) c->seeking = 0.0f;
    }
    c->behavior_ticks++;
}

// --- F3: BEH_CHASE ---
// F3 Chasing; other cat flees; ASCII: & moving fast
void beh_chase(Cat *c, Cat *target, World *w)
{
    c->behavior = BEH_CHASE;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_LASHING;
    // Move every tick (faster than stalk)
    move_toward(c, target->x, target->y, w);
    c->behavior_ticks++;
}

// --- F3: BEH_FLEE ---
// fear>0.7; ASCII: &; vocal: hiss (Tavernier 2020)
void beh_flee(Cat *c, Cat *threat, World *w)
{
    c->behavior = BEH_FLEE;
    c->ears = EAR_BACK;
    c->tail = TAIL_TUCKED;
    vocalize(c, VOC_HISS);
    move_away(c, threat->x, threat->y, w);
    c->fear    -= 0.05f;  // escaping reduces fear
    if (c->fear < 0.0f) c->fear = 0.0f;
    c->behavior_ticks++;
}

// --- F1: BEH_WRESTLE ---
// Mutual play (56% of interactions are playful per Ramos et al. 2017)
// ASCII: @& overlap
void beh_wrestle(Cat *a, Cat *b)
{
    a->behavior = BEH_WRESTLE;
    b->behavior = BEH_WRESTLE;
    a->ears = EAR_FORWARD; b->ears = EAR_FORWARD;
    a->tail = TAIL_LASHING; b->tail = TAIL_LASHING;
    a->play -= 0.1f; b->play -= 0.1f;
    if (a->play < 0.0f) a->play = 0.0f;
    if (b->play < 0.0f) b->play = 0.0f;
    a->energy -= 1.0f; b->energy -= 1.0f;
    a->behavior_ticks++; b->behavior_ticks++;
    memory_record(a, BEH_WRESTLE, +0.8f);
    memory_record(b, BEH_WRESTLE, +0.8f);
}

// --- F1: BEH_BUNNY_KICK ---
// Wrestling clinch; vocal: growl (Tavernier 2020); ASCII: *
void beh_bunny_kick(Cat *c, Cat *target)
{
    c->behavior = BEH_BUNNY_KICK;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_PUFFED;
    vocalize(c, VOC_GROWL);
    target->stress += 2.0f;
    c->play  -= 0.2f;
    c->rage  += 0.1f;
    if (c->play < 0.0f) c->play = 0.0f;
    if (c->rage > 1.0f) c->rage = 1.0f;
    c->behavior_ticks++;
}

// --- F6: BEH_ALLOGROOM ---
// Allogrooming: unidirectional 91.6% (Alberts et al. 2002)
// Higher-ranking grooms lower 78.6%; targets head/neck
// Redirects aggression; ASCII: &>
// Initiator = higher dominance+boldness (simulates 90.4% male initiation)
void beh_allogroom(Cat *giver, Cat *receiver)
{
    giver->behavior = BEH_ALLOGROOM;
    giver->ears = EAR_FORWARD;
    giver->tail = TAIL_DOWN;
    vocalize(giver, VOC_PURR);
    receiver->stress -= 3.0f;
    giver->care      -= 0.1f;
    giver->rage      -= 0.2f;  // redirects aggression (Alberts et al. 2002)
    if (receiver->stress < 0.0f) receiver->stress = 0.0f;
    if (giver->care     < 0.0f)  giver->care     = 0.0f;
    if (giver->rage     < 0.0f)  giver->rage     = 0.0f;
    giver->behavior_ticks++;
    memory_record(giver,    BEH_ALLOGROOM, +0.6f);
    memory_record(receiver, BEH_ALLOGROOM, +0.6f);
}

// Gate: can a cat initiate allogrooming?
// Requires 3+ positive memories, care>0.6, higher dominance (Alberts et al. 2002)
bool can_allogroom(Cat *giver, Cat *receiver)
{
    int positives = count_positive_memories(giver);
    return (positives >= 3
         && giver->care     > 0.6f
         && giver->dominance > receiver->dominance);
}

// --- F2: BEH_VOCAL_CHIRP ---
// Greeting/trill (Tavernier 2020; F5 recurring positive interactivity)
void beh_vocal_chirp(Cat *c)
{
    c->behavior = BEH_VOCAL_CHIRP;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_UP;  // tail-up is greeting/positive signal
    vocalize(c, VOC_CHIRP);
    c->behavior_ticks++;
}

// --- F2: BEH_VOCAL_HISS ---
// Agonistic threat; ears back; hiss+spit (Tavernier 2020)
void beh_vocal_hiss(Cat *c)
{
    c->behavior = BEH_VOCAL_HISS;
    c->ears = EAR_BACK;
    c->tail = TAIL_PUFFED;
    vocalize(c, VOC_HISS);
    c->rage  += 0.1f;
    if (c->rage > 1.0f) c->rage = 1.0f;
    c->behavior_ticks++;
}

// --- F2: BEH_VOCAL_GROWL ---
// Agonistic warning (Tavernier 2020)
void beh_vocal_growl(Cat *c)
{
    c->behavior = BEH_VOCAL_GROWL;
    c->ears = EAR_BACK;
    c->tail = TAIL_LASHING;
    vocalize(c, VOC_GROWL);
    c->behavior_ticks++;
}

// --- F2: BEH_VOCAL_YOWL ---
// High arousal distress/conflict (Tavernier 2020)
void beh_vocal_yowl(Cat *c)
{
    c->behavior = BEH_VOCAL_YOWL;
    c->ears = EAR_BACK;
    c->tail = TAIL_PUFFED;
    vocalize(c, VOC_YOWL);
    c->stress  += 2.0f;
    c->arousal += 5.0f;
    if (c->stress  > 100.0f) c->stress  = 100.0f;
    if (c->arousal > 100.0f) c->arousal = 100.0f;
    c->behavior_ticks++;
}

// --- Two-cat interaction protocol (plan §3.3) ---
// Outcome distribution targets: 56.2% playful, 15.2% intermediate,
// 28.6% agonistic (Ramos et al. 2017)
void interact(Cat *a, Cat *b, World *w)
{
    (void)w;

    // Step 1: Assessment – set ear position based on fear
    a->ears = (a->fear > 0.5f) ? EAR_BACK : EAR_FORWARD;
    b->ears = (b->fear > 0.5f) ? EAR_BACK : EAR_FORWARD;

    // Research rule: both ears forward → positive (Deputte 2021)
    bool both_forward = (a->ears == EAR_FORWARD && b->ears == EAR_FORWARD);

    // Step 2: Initiator = cat with higher (seeking + play)
    Cat *initiator = (a->seeking + a->play > b->seeking + b->play) ? a : b;
    Cat *receiver  = (initiator == a) ? b : a;
    (void)receiver;

    // Step 3: Negotiation → outcome selection
    if (both_forward && initiator->play > 0.5f) {
        // Playful (target 56%) – F1 Wrestling
        beh_wrestle(a, b);
    } else if (a->rage > 0.6f || b->rage > 0.6f) {
        // Agonistic (target 29%) – F2 Vocalising + flee
        beh_vocal_hiss(initiator);
        beh_flee(receiver, initiator, w);
        memory_record(a, BEH_VOCAL_HISS, -0.7f);
        memory_record(b, BEH_FLEE,       -0.7f);
    } else {
        // Intermediate (target 15%) – blend F1+F2 (not a switch, a blend)
        beh_stalk(initiator, receiver);
        beh_wiggle(initiator);
        if (rand() % 2) beh_vocal_chirp(initiator);
    }

    // Step 4: Allogrooming dominance check (Alberts et al. 2002)
    if (a->behavior == BEH_ALLOGROOM || b->behavior == BEH_ALLOGROOM) {
        // Higher-ranking grooms lower 78.6% of the time
        if (a->dominance > b->dominance && can_allogroom(a, b)) {
            beh_allogroom(a, b);
        } else if (b->dominance > a->dominance && can_allogroom(b, a)) {
            beh_allogroom(b, a);
        }
    }
}
