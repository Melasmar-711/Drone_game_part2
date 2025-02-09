#include "Generator_functions.h"
#include "sig_handle.h"
#include "logger.h"

int main() {
    char* log_file = "../Logs/TargetsGenerator.log";
    signal(SIGUSR2, handle_reset_signal);
    signal(SIGINT, handle_stop_signal);

    log_message(log_file, INFO, "TargetsGenerator started successfully.");

    int fifo_id = 0;

    int fps_value;
    int MAX_X;
    int MAX_Y;

    get_int_from_json("../Game_Config.json", "MAX_X", &MAX_X);
    get_int_from_json("../Game_Config.json", "MAX_Y", &MAX_Y);
    get_int_from_json("../Game_Config.json", "FPS", &fps_value);

start:
    if (reset) {
        log_message(log_file, INFO, "TargetsGenerator reset.");
        fifo_id++;
        reset = false;
        get_int_from_json("../Game_Config.json", "MAX_X", &MAX_X);
        get_int_from_json("../Game_Config.json", "MAX_Y", &MAX_Y);
        get_int_from_json("../Game_Config.json", "FPS", &fps_value);
        usleep(10000);
    }

    // Seed random number generator
    srand(time(NULL) + 1);

    // Create FIFO
    int fd_target_generator_to_server = create_and_open_fifo("/tmp/target_generator_to_server_%d", fifo_id, O_WRONLY);

    int num_targets = MAX_TARGETS;
    int targets[MAX_TARGETS][2];

    // Generate random targets within specified boundaries, ensuring even positions
    for (int i = 0; i < num_targets; i++) {
        targets[i][0] = (rand() % ((MAX_X - 2) / 2)) * 2 + 2;  // X coordinate (even)
        targets[i][1] = (rand() % ((MAX_Y - 2) / 2)) * 2 + 2;  // Y coordinate (even)
    }

    // Send the targets array to the server
    ssize_t bytes_written = write(fd_target_generator_to_server, targets, sizeof(targets));
    if (bytes_written == -1) {
        perror("Error writing to FIFO");
        close(fd_target_generator_to_server);
        return 1;
    }

    printf("Generated and sent %d targets.\n", num_targets);

    while (!reset & !stop) {
        log_message(log_file, INFO, "TargetsGenerator running.");
        usleep(1000000 / fps_value);
    }

    if (reset) {
        close(fd_target_generator_to_server);
        goto start;
    }

    log_message(log_file, INFO, "TargetsGenerator shutting down.");
    close(fd_target_generator_to_server);
    return 0;
}