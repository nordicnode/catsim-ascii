#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <math.h>
#include "cat.h"
#include "world.h"
#include "behavior.h"
#include "fcm.h"
#include "render.h"

#define TICK_RATE_HZ   10
#define TICK_NS        (1000000000L / TICK_RATE_HZ)   
#define DT             (1.0f / TICK_RATE_HZ)          

static void sleep_until(struct timespec *next)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (next->tv_sec > now.tv_sec ||
        (next->tv_sec == now.tv_sec && next->tv_nsec > now.tv_nsec)) {
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, next, NULL);
    }
    next->tv_nsec += TICK_NS;
    if (next->tv_nsec >= 1000000000L) {
        next->tv_nsec -= 1000000000L;
        next->tv_sec++;
    }
}

int main(void)
{
    srand((unsigned)time(NULL));

    World world;
    world_load(&world, "data/map_default.txt");

    Cat milo, luna;
    cat_init(&milo, "Milo",  10,  5,  0.8f, 0.7f, 0.3f, 0.9f, 0.2f, +0.3f);
    cat_init(&luna, "Luna",  60, 15,  0.4f, 0.5f, 0.2f, 0.6f, 0.6f, -0.3f);

    render_init();

    int  global_tick   = 0;
    int  sim_time_sec  = 0;

    struct timespec next_tick;
    clock_gettime(CLOCK_MONOTONIC, &next_tick);

    while (1) {
        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;

        update_physiology(&milo, DT, global_tick);
        update_physiology(&luna, DT, global_tick);

        perceive(&milo, &luna, &world);
        perceive(&luna, &milo, &world);

        float ddx = (float)(luna.x - milo.x);
        float ddy = (float)(luna.y - milo.y);
        float dist = sqrtf(ddx*ddx + ddy*ddy);

        if (!milo.sleeping) {
            FCMOutput fcm_m = fcm_evaluate(&milo, &luna, dist);
            Behavior  beh_m = fcm_select_behavior(&fcm_m, &milo);
            milo.behavior = beh_m;
        }
        if (!luna.sleeping) {
            FCMOutput fcm_l = fcm_evaluate(&luna, &milo, dist);
            Behavior  beh_l = fcm_select_behavior(&fcm_l, &luna);
            luna.behavior = beh_l;
        }

        // --- Execute behaviors ---
        if (dist < 3.0f && !milo.sleeping && !luna.sleeping) {
            interact(&milo, &luna, &world);
        } else {
            // Execute individual behaviors
            switch (milo.behavior) {
                case BEH_SLEEP:      beh_sleep(&milo);                   break;
                case BEH_GROOM_SELF: beh_groom_self(&milo);              break;
                case BEH_STALK:      beh_stalk(&milo, &luna, &world);    break;
                case BEH_WIGGLE:     beh_wiggle(&milo);                  break;
                case BEH_POUNCE:     beh_pounce(&milo, &luna, &world);   break;
                case BEH_CHASE:      beh_chase(&milo, &luna, &world);    break;
                case BEH_FLEE:       beh_flee(&milo, &luna, &world);     break;
                case BEH_WRESTLE:
                case BEH_ALLOGROOM:
                    move_toward(&milo, luna.x, luna.y, &world); // Approach target
                    milo.behavior_ticks++;
                    break;
                case BEH_VOCAL_CHIRP: beh_vocal_chirp(&milo);            break;
                case BEH_VOCAL_HISS:  beh_vocal_hiss(&milo);             break;
                case BEH_VOCAL_GROWL: beh_vocal_growl(&milo);            break;
                case BEH_VOCAL_YOWL:  beh_vocal_yowl(&milo);             break;
                default: break;
            }
            switch (luna.behavior) {
                case BEH_SLEEP:      beh_sleep(&luna);                   break;
                case BEH_GROOM_SELF: beh_groom_self(&luna);              break;
                case BEH_STALK:      beh_stalk(&luna, &milo, &world);    break;
                case BEH_WIGGLE:     beh_wiggle(&luna);                  break;
                case BEH_POUNCE:     beh_pounce(&luna, &milo, &world);   break;
                case BEH_CHASE:      beh_chase(&luna, &milo, &world);    break;
                case BEH_FLEE:       beh_flee(&luna, &milo, &world);     break;
                case BEH_WRESTLE:
                case BEH_ALLOGROOM:
                    move_toward(&luna, milo.x, milo.y, &world); // Approach target
                    luna.behavior_ticks++;
                    break;
                case BEH_VOCAL_CHIRP: beh_vocal_chirp(&luna);            break;
                case BEH_VOCAL_HISS:  beh_vocal_hiss(&luna);             break;
                case BEH_VOCAL_GROWL: beh_vocal_growl(&luna);            break;
                case BEH_VOCAL_YOWL:  beh_vocal_yowl(&luna);             break;
                default: break;
            }
        }

        // --- Render ---
        erase();
        render_world(&world);
        render_cats(&milo, &luna);
        render_statusbar(&milo, &luna, sim_time_sec);
        render_flush();

        global_tick++;
        if (global_tick % TICK_RATE_HZ == 0) sim_time_sec++;

        if (global_tick % 864000 == 0) {
            milo.active_ticks = 0;
            luna.active_ticks = 0;
        }

        sleep_until(&next_tick);
    }

    render_cleanup();
    return 0;
}
