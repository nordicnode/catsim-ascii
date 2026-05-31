#pragma once
#include "cat.h"
#include "world.h"

// All 14 ethogram behaviors as per implementation plan §3.1
// Each maps to an F-factor from Ramos et al. 2017

// F4 Non-interacting
void beh_sleep(Cat *c);       // energy<30; ASCII: c
void beh_groom_self(Cat *c);  // stress>30; ASCII: & licking

// F3 Chasing – prey/chase sequence
void beh_stalk(Cat *c, Cat *target);   // seeking>0.6; ASCII: ~& crouched
void beh_wiggle(Cat *c);               // pre-pounce butt-wiggle; ASCII: & shaking
void beh_pounce(Cat *c, Cat *target, World *w);  // distance<2; ASCII: @
void beh_chase(Cat *c, Cat *target, World *w);   // other flees; ASCII: & fast
void beh_flee(Cat *c, Cat *threat, World *w);    // fear>0.7; ASCII: &

// F1 Wrestling
void beh_wrestle(Cat *a, Cat *b);     // mutual play; ASCII: @& overlap
void beh_bunny_kick(Cat *c, Cat *target);  // wrestling; ASCII: *

// F6 Prolonged interactivity
void beh_allogroom(Cat *giver, Cat *receiver);  // care>0.6; ASCII: &>

// F2 Vocalising
void beh_vocal_chirp(Cat *c);   // greeting; chirp/trill (Tavernier 2020)
void beh_vocal_hiss(Cat *c);    // threat;   hiss+spit  (Tavernier 2020)
void beh_vocal_growl(Cat *c);   // agonistic; growl      (Tavernier 2020)
void beh_vocal_yowl(Cat *c);    // high arousal; yowl    (Tavernier 2020)

// Allogrooming gate (Alberts et al. 2002)
bool can_allogroom(Cat *giver, Cat *receiver);

// Two-cat interaction protocol (plan §3.3)
void interact(Cat *a, Cat *b, World *w);
