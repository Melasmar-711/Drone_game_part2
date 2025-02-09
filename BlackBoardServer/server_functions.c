#include "server_functions.h"



long current_time_in_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}



int get_max_fd(int fds[], int num_fds) {
    int max_fd = fds[0];
    for (int i = 1; i < num_fds; i++) {
        if (fds[i] > max_fd) {
            max_fd = fds[i];
        }
    }
    return max_fd;
}


void handle_keyboard_input(int fd, KeyboardInput *input, KeyboardInput *prev_input, ServerState *state) {
    ssize_t bytes_read = read(fd, input, sizeof(KeyboardInput));
    if (bytes_read == sizeof(KeyboardInput)) {
        if (input->quit) {
            printf("Quit signal received. Shutting down.\n");
            exit(0);
        }
        if (memcmp(input, prev_input, sizeof(KeyboardInput))) {
            state->input_x_force = input->force_x;
            state->input_y_force = input->force_y;
            *prev_input = *input;
        }
    }
}

void send_state_to_dynamics(int fd, KeyboardInput *input, KeyboardInput *prev_input, ServerState *state) {
    if (memcmp(input, prev_input, sizeof(KeyboardInput))) {
        write(fd, state, sizeof(ServerState));
    }
}

void handle_dynamics_input(int fd, ServerState *state) {
    ssize_t bytes_read = read(fd, state, sizeof(ServerState));
    if (bytes_read == sizeof(ServerState)) {
        printf("Updated state received from DroneDynamics: %f\n", state->drone_x);
    }
}



