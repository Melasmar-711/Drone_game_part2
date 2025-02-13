
#include"sig_handle.h"


volatile bool is_paused = false;
volatile bool reset = false;
volatile bool stop = false;





// Signal handler to toggle pause state
void handle_pause_signal(int sig) {
    if (sig == SIGUSR1) {
        is_paused = !is_paused;
        printf("Pause state toggled: %s\n", is_paused ? "Paused" : "Running");
    }
}


void handle_reset_signal(int sig) {
    if (sig == SIGUSR2) {
        reset = true;
        printf("Game reset: %s\n", reset ? "reset" : "Running");
    }
}


void handle_stop_signal(int sig) {
    if (sig == SIGINT) {
        
        stop = true;
        printf("Pause state toggled: %s\n", stop ? "stopped" : "Running");
    }
}

