#include <math.h>
#include "fcm.h"

// Weight matrix per implementation plan (do NOT randomize)
// All variation comes from physiology and memory, not noise.
FCMOutput fcm_evaluate(const Cat *c, const Cat *other, float other_dist)
{
    (void)other;  // reserved for future cross-cat FCM coupling
    FCMOutput o;

    float energy_norm     = c->energy / 100.0f;
    float other_dist_norm = other_dist / 80.0f;  // normalize to world width
    float dominance_abs   = fabsf(c->dominance);
    bool  other_ears_fwd  = (other->ears == EAR_FORWARD) ? 1.0f : 0.0f;
    (void)other_ears_fwd;  // factored into FCM via play/fear from perceive()

    // F1 Wrestling/Play (Ramos et al. 2017)
    o.f1_wrestle = tanhf(
         0.3f * c->play
       + 0.2f * c->seeking
       - 0.4f * c->fear
       + 0.1f * c->boldness
       - 0.2f * energy_norm
    );

    // F2 Vocalising
    o.f2_vocal = tanhf(
         0.4f * c->rage
       + 0.3f * c->fear
       + 0.2f * c->stress / 100.0f
       - 0.1f * c->care
    );

    // F3 Chasing
    o.f3_chase = tanhf(
         0.5f * c->seeking
       + 0.3f * c->play
       - 0.3f * other_dist_norm
    );

    // F4 Non-interacting
    o.f4_nonint = tanhf(
       - 0.6f * energy_norm
       + 0.3f * c->stress / 100.0f
       + 0.2f * c->fear
    );

    // F5 Recurring interactivity
    o.f5_recur = tanhf(
         0.3f * c->curiosity
       + 0.2f * c->play
       - 0.2f * dominance_abs
    );

    // F6 Prolonged interactivity (allogrooming, proximity)
    o.f6_prolong = tanhf(
         0.5f * c->care
       + 0.3f * c->sociability
       - 0.4f * c->rage
    );

    return o;
}

// Select behavior: high-fear/rage overrides checked FIRST, then argmax(F1..F6)
// Fear override before argmax prevents terrified cats from attempting to groom
Behavior fcm_select_behavior(const FCMOutput *fcm, const Cat *c)
{
    // Low energy → forced non-interaction
    if (c->energy < 20.0f) return BEH_IDLE;
    // Sleeping is handled by physiology, not FCM
    if (c->sleeping)       return BEH_SLEEP;

    // High fear/rage overrides evaluated BEFORE argmax so they cannot be
    // outscored by f6_prolong (prevents terrified cats attempting to groom)
    if (c->fear > 0.7f) {
        return (c->rage > 0.4f) ? BEH_VOCAL_HISS : BEH_FLEE;
    }

    float best = 0.3f;  // threshold – prevents flailing on all-low activations
    Behavior sel = BEH_IDLE;

#define CHECK(score, beh) if ((score) > best) { best = (score); sel = (beh); }
    CHECK(fcm->f1_wrestle, BEH_WRESTLE);
    CHECK(fcm->f2_vocal,   BEH_VOCAL_GROWL);
    CHECK(fcm->f3_chase,   BEH_STALK);
    CHECK(fcm->f4_nonint,  BEH_GROOM_SELF);
    CHECK(fcm->f5_recur,   BEH_VOCAL_CHIRP);
    CHECK(fcm->f6_prolong, BEH_ALLOGROOM);
#undef CHECK

    // High seeking → prefer stalk sequence over raw wrestling
    if (c->seeking > 0.6f && sel == BEH_WRESTLE) {
        return BEH_STALK;
    }

    return sel;
}
