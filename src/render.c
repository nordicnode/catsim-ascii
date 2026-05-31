#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include "render.h"
#include "cat.h"
#include "world.h"

// Color pair IDs
#define COLOR_CAT_A   1
#define COLOR_CAT_B   2
#define COLOR_WRESTLE 3
#define COLOR_WALL    4
#define COLOR_STATUS  5
#define COLOR_FOOD    6

void render_init(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);  // non-blocking getch
    curs_set(0);

    if (has_colors()) {
        start_color();
        init_pair(COLOR_CAT_A,   COLOR_YELLOW, COLOR_BLACK);  // Milo
        init_pair(COLOR_CAT_B,   COLOR_CYAN,   COLOR_BLACK);  // Luna
        init_pair(COLOR_WRESTLE, COLOR_RED,    COLOR_BLACK);  // wrestling flash
        init_pair(COLOR_WALL,    COLOR_WHITE,  COLOR_BLACK);
        init_pair(COLOR_STATUS,  COLOR_GREEN,  COLOR_BLACK);
        init_pair(COLOR_FOOD,    COLOR_MAGENTA,COLOR_BLACK);
    }
}

void render_cleanup(void)
{
    endwin();
}

void render_world(const World *w)
{
    for (int y = 0; y < WORLD_H; y++) {
        for (int x = 0; x < WORLD_W; x++) {
            char t = world_tile(w, x, y);
            if (t == '#') {
                attron(COLOR_PAIR(COLOR_WALL));
                mvaddch(y, x, '#');
                attroff(COLOR_PAIR(COLOR_WALL));
            } else if (t == 'o') {
                attron(COLOR_PAIR(COLOR_FOOD));
                mvaddch(y, x, 'o');
                attroff(COLOR_PAIR(COLOR_FOOD));
            } else if (t == '~') {
                mvaddch(y, x, '~');  // bed
            } else {
                mvaddch(y, x, ' ');
            }
        }
    }
}

// ASCII character per behavior state (plan §7)
static char cat_char(const Cat *c)
{
    switch (c->behavior) {
        case BEH_SLEEP:      return 'c';
        case BEH_POUNCE:     return '@';
        case BEH_WRESTLE:    return '@';
        case BEH_BUNNY_KICK: return '*';
        case BEH_WIGGLE:     return '&';  // shaking
        default:             return '&';
    }
}

// Draw directional markers for a single cat.
// tail_lash: ~ is placed one cell behind the cat in its movement direction.
// ear_back:  > is placed one cell ahead of the cat (showing threat direction).
// Falls back to x-1 / x+1 if dx==0 && dy==0 (cat hasn't moved yet).
static void render_cat_markers(const Cat *c)
{
    // --- Tail lashing marker ---
    if (c->tail == TAIL_LASHING) {
        // Behind = opposite of movement direction
        int tail_dx = (c->dx != 0) ? -c->dx : -1;  // default behind = left
        int tx = c->x + tail_dx;
        int ty = c->y + ((c->dy != 0) ? -c->dy : 0);
        if (tx >= 0 && tx < WORLD_W && ty >= 0 && ty < WORLD_H)
            mvaddch(ty, tx, '~');
    }

    // --- Ear back / flat threat marker ---
    if (c->ears == EAR_BACK || c->ears == EAR_FLAT) {
        // Ahead = in movement direction (facing the threat)
        int ear_dx = (c->dx != 0) ? c->dx : 1;   // default ahead = right
        int ex = c->x + ear_dx;
        int ey = c->y + ((c->dy != 0) ? c->dy : 0);
        if (ex >= 0 && ex < WORLD_W && ey >= 0 && ey < WORLD_H)
            mvaddch(ey, ex, '>');
    }
}

void render_cats(const Cat *a, const Cat *b)
{
    bool wrestling = (a->behavior == BEH_WRESTLE && b->behavior == BEH_WRESTLE);

    // Cat A
    if (!wrestling) {
        attron(COLOR_PAIR(COLOR_CAT_A));
        if (a->behavior != BEH_SLEEP) attron(A_BOLD);
        mvaddch(a->y, a->x, cat_char(a));
        render_cat_markers(a);
        attroff(A_BOLD);
        attroff(COLOR_PAIR(COLOR_CAT_A));
    }

    // Cat B
    if (!wrestling) {
        attron(COLOR_PAIR(COLOR_CAT_B));
        if (b->behavior != BEH_SLEEP) attron(A_BOLD);
        mvaddch(b->y, b->x, cat_char(b));
        render_cat_markers(b);
        attroff(A_BOLD);
        attroff(COLOR_PAIR(COLOR_CAT_B));
    } else {
        // Both in same cell → red flash *
        attron(COLOR_PAIR(COLOR_WRESTLE) | A_BOLD | A_BLINK);
        mvaddch(a->y, a->x, '*');
        attroff(COLOR_PAIR(COLOR_WRESTLE) | A_BOLD | A_BLINK);
    }

    // Allogrooming: show > at giver's facing cell
    if (a->behavior == BEH_ALLOGROOM) {
        int ear_dx = (a->dx != 0) ? a->dx : 1;
        attron(COLOR_PAIR(COLOR_CAT_A) | A_BOLD);
        mvaddch(a->y, a->x + ear_dx, '>');
        attroff(COLOR_PAIR(COLOR_CAT_A) | A_BOLD);
    }
    if (b->behavior == BEH_ALLOGROOM) {
        int ear_dx = (b->dx != 0) ? b->dx : 1;
        attron(COLOR_PAIR(COLOR_CAT_B) | A_BOLD);
        mvaddch(b->y, b->x + ear_dx, '>');
        attroff(COLOR_PAIR(COLOR_CAT_B) | A_BOLD);
    }
}

const char *vocal_name(VocalType v)
{
    switch (v) {
        case VOC_PURR:    return "purr";
        case VOC_MEOW:    return "meow";
        case VOC_HISS:    return "hiss";
        case VOC_GROWL:   return "growl";
        case VOC_CHIRP:   return "chirp";
        case VOC_CHATTER: return "chatter";
        case VOC_YOWL:    return "yowl";
        case VOC_SNARL:   return "snarl";
        case VOC_SPIT:    return "spit";
        case VOC_MOAN:    return "moan";
        case VOC_SQUEAK:  return "squeak";
        default:          return "";
    }
}

const char *behavior_name(Behavior b)
{
    switch (b) {
        case BEH_SLEEP:       return "SLEEP";
        case BEH_GROOM_SELF:  return "GROOM";
        case BEH_STALK:       return "STALK";
        case BEH_WIGGLE:      return "WIGGLE";
        case BEH_POUNCE:      return "POUNCE";
        case BEH_BUNNY_KICK:  return "BKICK";
        case BEH_CHASE:       return "CHASE";
        case BEH_FLEE:        return "FLEE";
        case BEH_WRESTLE:     return "WRESTLE";
        case BEH_ALLOGROOM:   return "ALLGRM";
        case BEH_VOCAL_CHIRP: return "CHIRP";
        case BEH_VOCAL_HISS:  return "HISS";
        case BEH_VOCAL_GROWL: return "GROWL";
        case BEH_VOCAL_YOWL:  return "YOWL";
        default:              return "IDLE";
    }
}

// Status bar: lines 22-24 per plan §7
void render_statusbar(const Cat *a, const Cat *b, int sim_time_sec)
{
    int mm = (sim_time_sec / 60) % 60;
    int ss = sim_time_sec % 60;
    int hh = sim_time_sec / 3600;

    // Line 22: time + state
    attron(COLOR_PAIR(COLOR_STATUS));
    mvprintw(WORLD_H, 0,
        "Time %02d:%02d:%02d | %s: E%2.0f S%2.0f %-6s | %s: E%2.0f S%2.0f %-6s | Rel: %+.1f",
        hh, mm, ss,
        a->name, a->energy, a->stress, behavior_name(a->behavior),
        b->name, b->energy, b->stress, behavior_name(b->behavior),
        a->dominance - b->dominance);

    // Line 23: last vocal event
    char vocal_a[64] = "", vocal_b[64] = "";
    if (a->vocal != VOC_NONE)
        snprintf(vocal_a, sizeof(vocal_a), "%s: %s", a->name, vocal_name(a->vocal));
    if (b->vocal != VOC_NONE)
        snprintf(vocal_b, sizeof(vocal_b), "%s: %s", b->name, vocal_name(b->vocal));

    mvprintw(WORLD_H + 1, 0, "%-36s %-36s", vocal_a, vocal_b);

    // Line 24: ear state note (Deputte 2021 rule)
    const char *ear_note = "";
    if (a->ears == EAR_FORWARD && b->ears == EAR_FORWARD)
        ear_note = "Both ears forward -> positive outcome predicted";
    else if (a->ears == EAR_BACK || b->ears == EAR_BACK)
        ear_note = "Ears back -> negative/distance outcome predicted";
    mvprintw(WORLD_H + 2, 0, "%-79s", ear_note);

    attroff(COLOR_PAIR(COLOR_STATUS));
}

void render_flush(void)
{
    refresh();
}
