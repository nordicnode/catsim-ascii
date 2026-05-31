#pragma once
#include "cat.h"
#include "world.h"

// Movement utility exposed for pathing
void move_toward(Cat *c, int tx, int ty, World *w);
void move_away(Cat *c, int tx, int ty, World *w);

// F4 Non-interacting
void beh_sleep(Cat *c);       
void beh_groom_self(Cat *c);  

// F3 Chasing – prey/chase sequence
void beh_stalk(Cat *c, Cat *target, World *w);   // Updated to include World*
void beh_wiggle(Cat *c);               
void beh_pounce(Cat *c, Cat *target, World *w);  
void beh_chase(Cat *c, Cat *target, World *w);   
void beh_flee(Cat *c, Cat *threat, World *w);    

// F1 Wrestling
void beh_wrestle(Cat *a, Cat *b);     
void beh_bunny_kick(Cat *c, Cat *target);  

// F6 Prolonged interactivity
void beh_allogroom(Cat *giver, Cat *receiver);  

// F2 Vocalising
void beh_vocal_chirp(Cat *c);   
void beh_vocal_hiss(Cat *c);    
void beh_vocal_growl(Cat *c);   
void beh_vocal_yowl(Cat *c);    

bool can_allogroom(Cat *giver, Cat *receiver);
void interact(Cat *a, Cat *b, World *w);
