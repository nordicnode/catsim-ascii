#pragma once
#include "cat.h"

// Fuzzy Cognitive Map – 6 nodes matching the 6 interaction factors
// (Ramos et al. 2017: F1 Wrestling, F2 Vocalising, F3 Chasing,
//  F4 Non-interacting, F5 Recurring, F6 Prolonged interactivity)
typedef struct {
    float f1_wrestle;
    float f2_vocal;
    float f3_chase;
    float f4_nonint;
    float f5_recur;
    float f6_prolong;
} FCMOutput;

FCMOutput fcm_evaluate(const Cat *c, const Cat *other, float other_dist);
Behavior  fcm_select_behavior(const FCMOutput *fcm, const Cat *c);
