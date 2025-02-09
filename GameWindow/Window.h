
#include <math.h>
#include"shared.h"
#include"sig_handle.h"
#include<ncurses.h>



extern bool just_got_reset;



#define DELAY 1000 // Delay in microseconds (controls frame rate)

// Function prototypes
void init_ncurses();
void draw_borders(int MAX_X, int MAX_Y);
void draw_simulation(ServerState *prev_state, ServerState *current_state,int *flags);   