#ifndef KEYBOARD
#define KEYBOARD

#include <ncurses.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<string.h>
#include<signal.h>
#include"shared.h"




#define DELAY 500000 // Delay in microseconds

#define Pause_or_Continue 10
#define stop  11
#define Continue 12
#define Re_set 20

// Define the structure in a shared header file (e.g., `shared.h`)
typedef struct {
    int force_x; // Force in the x-direction
    int force_y; // Force in the y-direction
    int quit;    // Flag to indicate if the user wants to quit
} KeyboardInput;




void init_ncurses();
void process_input(KeyboardInput *input);
void draw_keyboard_layout(KeyboardInput *input) ;


#endif