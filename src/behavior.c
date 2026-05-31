#include <stdlib.h>
#include <math.h>
#include "behavior.h"
#include "world.h"

// --- Utility: clamp move to non-wall cell and record direction ---
void move_toward(Cat *c, int tx, int ty, World *w)
{
    int nx = c->x + (tx > c->x ? 1 : tx < c->x ? -1 : 0);
    int ny = c->y + (ty > c->y ? 1 : ty < c->y ? -1 : 0);
    if (!world_is_wall(w, nx, ny)) {
        if (nx != c->x || ny != c->y) {
            c->dx = nx - c->x;
            c->dy = ny - c->y;
        }
        c->x = nx; c->y = ny;
    }
}

void move_away(Cat *c, int tx, int ty, World *w)
{
    int nx = c->x - (tx > c->x ? 1 : tx < c->x ? -1 : 0);
    int ny = c->y - (ty > c->y ? 1 : ty < c->y ? -1 : 0);
    if (!world_is_wall(w, nx, ny)) {
        if (nx != c->x || ny != c->y) {
            c->dx = nx - c->x;
            c->dy = ny - c->y;
        }
        c->x = nx; c->y = ny;
    }
}

// --- F4: BEH_SLEEP ---
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
void beh_stalk(Cat *c, Cat *target, World *w)
{
    c->behavior = BEH_STALK;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_DOWN;
    
    float ddx = (float)(target->x - c->x);
    float ddy = (float)(target->y - c->y);
    float dist = sqrtf(ddx*ddx + ddy*ddy);
    
    if (dist > 3.0f) {
        if (c->behavior_ticks % 3 == 0) {
            move_toward(c, target->x, target->y, w);
        }
    }
    if (c->seeking > 0.8f && c->behavior_ticks > 5) {
        vocalize(c, VOC_CHATTER);
    }
    c->behavior_ticks++;
}

// --- F3: BEH_WIGGLE ---
void beh_wiggle(Cat *c)
{
    c->behavior = BEH_WIGGLE;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_LASHING;
    c->arousal += 0.1f;
    c->behavior_ticks++;
}

// --- F3: BEH_POUNCE ---
void beh_pounce(Cat *c, Cat *target, World *w)
{
    c->behavior = BEH_POUNCE;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_LASHING;
    move_toward(c, target->x, target->y, w);
    if (c->x == target->x && c->y == target->y) {
        c->play    += 0.3f;
        c->seeking -= 0.4f;
        if (c->seeking < 0.0f) c->seeking = 0.0f;
    }
    c->behavior_ticks++;
}

// --- F3: BEH_CHASE ---
void beh_chase(Cat *c, Cat *target, World *w)
{
    c->behavior = BEH_CHASE;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_LASHING;
    move_toward(c, target->x, target->y, w);
    c->behavior_ticks++;
}

// --- F3: BEH_FLEE ---
void beh_flee(Cat *c, Cat *threat, World *w)
{
    c->behavior = BEH_FLEE;
    c->ears = EAR_BACK;
    c->tail = TAIL_TUCKED;
    vocalize(c, VOC_HISS);
    move_away(c, threat->x, threat->y, w);
    c->fear    -= 0.05f;  
    if (c->fear < 0.0f) c->fear = 0.0f;
    c->behavior_ticks++;
}

// --- F1: BEH_WRESTLE ---
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
void beh_allogroom(Cat *giver, Cat *receiver)
{
    giver->behavior = BEH_ALLOGROOM;
    receiver->behavior = BEH_IDLE; // Receiver stands relaxed
    giver->ears = EAR_FORWARD;
    giver->tail = TAIL_DOWN;
    vocalize(giver, VOC_PURR);
    receiver->stress -= 3.0f;
    giver->care      -= 0.1f;
    giver->rage      -= 0.2f;  
    if (receiver->stress < 0.0f) receiver->stress = 0.0f;
    if (giver->care     < 0.0f)  giver->care     = 0.0f;
    if (giver->rage     < 0.0f)  giver->rage     = 0.0f;
    giver->behavior_ticks++;
    memory_record(giver,    BEH_ALLOGROOM, +0.6f);
    memory_record(receiver, BEH_ALLOGROOM, +0.6f);
}

bool can_allogroom(Cat *giver, Cat *receiver)
{
    int positives = count_positive_memories(giver);
    return (positives >= 3
         && giver->care     > 0.6f
         && giver->dominance > receiver->dominance);
}

// --- F2: BEH_VOCAL_CHIRP ---
void beh_vocal_chirp(Cat *c)
{
    c->behavior = BEH_VOCAL_CHIRP;
    c->ears = EAR_FORWARD;
    c->tail = TAIL_UP;  
    vocalize(c, VOC_CHIRP);
    c->behavior_ticks++;
}

// --- F2: BEH_VOCAL_HISS ---
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
void beh_vocal_growl(Cat *c)
{
    c->behavior = BEH_VOCAL_GROWL;
    c->ears = EAR_BACK;
    c->tail = TAIL_LASHING;
    vocalize(c, VOC_GROWL);
    c->behavior_ticks++;
}

// --- F2: BEH_VOCAL_YOWL ---
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

// --- Two-cat interaction protocol ---
void interact(Cat *a, Cat *b, World *w)
{
    // Cache original FCM intended states before negotiation overwrites them
    Behavior orig_a_beh = a->behavior;
    Behavior orig_b_beh = b->behavior;

    // Step 1: Assessment
    a->ears = (a->fear > 0.5f) ? EAR_BACK : EAR_FORWARD;
    b->ears = (b->fear > 0.5f) ? EAR_BACK : EAR_FORWARD;

    bool both_forward = (a->ears == EAR_FORWARD && b->ears == EAR_FORWARD);

    // Step 2: Initiator
    Cat *initiator = (a->seeking + a->play > b->seeking + b->play) ? a : b;
    Cat *receiver  = (initiator == a) ? b : a;

    // Step 3: Negotiation / Primary selection
    if (both_forward && initiator->play > 0.5f) {
        beh_wrestle(a, b);
    } else if (a->rage > 0.6f || b->rage > 0.6f) {
        beh_vocal_hiss(initiator);
        beh_flee(receiver, initiator, w);
        memory_record(a, BEH_VOCAL_HISS, -0.7f);
        memory_record(b, BEH_FLEE,       -0.7f);
    } else {
        beh_stalk(initiator, receiver, w);
        beh_wiggle(initiator);
        if (rand() % 2) beh_vocal_chirp(initiator);
    }

    // Step 4: Allogrooming priority bypass check
    if (orig_a_beh == BEH_ALLOGROOM || orig_b_beh == BEH_ALLOGROOM) {
        if (a->dominance > b->dominance && can_allogroom(a, b)) {
            beh_allogroom(a, b);
        } else if (b->dominance > a->dominance && can_allogroom(b, a)) {
            beh_allogroom(b, a);
        }
    }
}
