#ifndef SIGHANDLE
#define SIGHANDLE

#include <signal.h>
#include<stdbool.h>
#include<stdio.h>



extern volatile bool is_paused;
extern volatile bool reset ;
extern volatile bool stop ;



void handle_pause_signal(int sig);
void handle_reset_signal(int sig);
void handle_stop_signal(int sig) ;



#endif