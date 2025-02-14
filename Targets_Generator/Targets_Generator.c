#include "Generator_functions.h"
#include "sig_handle.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    printf("Starting TargetsGenerator...\n");

    if (argc >= 2) {
        fprintf(stderr, "Usage: %s <mode>\n", argv[1]);
        //return 1;
    }

    char *mode = argv[1];
    printf("Mode: %s\n", mode);

    if (strcmp(mode, "publisher") != 0 && strcmp(mode, "subscriber") != 0) {
        fprintf(stderr, "Invalid mode. Use 'publisher' or 'subscriber'.\n");
        while(1); // this will make it not log so the watchdog will know something is wrong and kill it along with the rest of the processes
    }

    int fd_target_generator; // to handle communication whether from the publisher or the subscriber

    char* log_file = "../Logs/TargetsGenerator.log";
    signal(SIGUSR2, handle_reset_signal);
    signal(SIGINT, handle_stop_signal);

    log_message(log_file, INFO, "TargetsGenerator started successfully.");
    printf("Logging initialized.\n");

    int fifo_id = 0;

    int fps_value;
    int MAX_X;
    int MAX_Y;

    get_int_from_json("../Game_Config.json", "MAX_X", &MAX_X);
    get_int_from_json("../Game_Config.json", "MAX_Y", &MAX_Y);
    get_int_from_json("../Game_Config.json", "FPS", &fps_value);

    int n_targets;
    get_int_from_json("../Game_Config.json", "num_of_targets", &n_targets);

    printf("Configuration loaded: MAX_X=%d, MAX_Y=%d, FPS=%d, num_of_targets=%d\n", MAX_X, MAX_Y, fps_value, n_targets);

    if (strcmp(mode, "publisher") == 0) {
        const char *fifo_path = "/tmp/target_generator_fifo_pub_%d";
        fd_target_generator = create_and_open_fifo(fifo_path, 0, O_WRONLY);
        if (fd_target_generator == -1) {
            perror("create_and_open_fifo");
            exit(EXIT_FAILURE);
        }
        printf("Publisher FIFO created and opened.\n");
    } 
    
    else if (strcmp(mode, "subscriber") == 0) {
        const char *fifo_path = "/tmp/target_generator_fifo_sub_%d"; 
        fd_target_generator = create_and_open_fifo(fifo_path, 0, O_RDONLY);
        if (fd_target_generator == -1) {
            perror("create_and_open_fifo");
            exit(EXIT_FAILURE);
        }
        printf("Subscriber FIFO created and opened.\n");
    }

start:
    if (reset) {
        log_message(log_file, INFO, "TargetsGenerator reset.");
        fifo_id++;
        reset = false;
        get_int_from_json("../Game_Config.json", "MAX_X", &MAX_X);
        get_int_from_json("../Game_Config.json", "MAX_Y", &MAX_Y);
        get_int_from_json("../Game_Config.json", "FPS", &fps_value);
        get_int_from_json("../Game_Config.json", "num_of_targets", &n_targets);

        printf("Reset configuration: MAX_X=%d, MAX_Y=%d, FPS=%d, num_of_targets=%d\n", MAX_X, MAX_Y, fps_value, n_targets);

        usleep(100000);
    }

    // Seed random number generator
    srand(time(NULL) + 1);

    if (strcmp(mode, "publisher") == 0) {
        while (1) {
            int num_targets = n_targets;
            int targets[n_targets][2];

            for (int i = 0; i < num_targets; i++) {
                targets[i][0] = (rand() % ((MAX_X - 2) / 2)) * 2 + 2;  // X coordinate (even)
                targets[i][1] = (rand() % ((MAX_Y - 2) / 2)) * 2 + 2;  // Y coordinate (even)
            }

            Targets_gen targets_to_publish = {0};
            targets_to_publish.num_targets = num_targets;
            for (int i = 0; i < num_targets; i++) {
                targets_to_publish.targets[i][0] = targets[i][0];
                targets_to_publish.targets[i][1] = targets[i][1];
            }

            ssize_t bytes_written = write(fd_target_generator, &targets_to_publish, sizeof(targets_to_publish));
            log_message(log_file, INFO, "TargetsGenerator publishing targets.");
            printf("Generated and sent %d targets.\n", num_targets);

            usleep(1000000);

            if (reset) {
                goto start;
            }

            if (stop) {
                close(fd_target_generator);
                log_message(log_file, INFO, "TargetsGenerator shutting down.");
                break;
            }
        }
    } else if (strcmp(mode, "subscriber") == 0) {
        int fd_target_generator_to_server = create_and_open_fifo("/tmp/target_generator_to_server_%d", fifo_id, O_WRONLY);
        Targets_gen targets_from_subscriber = {0};

        fd_set read_fds;
        struct timeval timeout;

        FD_ZERO(&read_fds);
        FD_SET(fd_target_generator, &read_fds);

        timeout.tv_sec = 0;  // Set timeout to 5 seconds
        timeout.tv_usec = 0;

        while (1) {
            if (reset) {
            printf("Reset signal received. Closing FIFO and restarting...\n");
            close(fd_target_generator_to_server);
            goto start;
            }

            if (stop) {
            printf("Stop signal received. Shutting down...\n");
            usleep(1000000);
            close(fd_target_generator_to_server);
            log_message(log_file, INFO, "TargetsGenerator shutting down.");
            exit(0);
            }

            printf("Waiting for data on FIFO...\n");
            int ret = select(fd_target_generator + 1, &read_fds, NULL, NULL, &timeout);


            
            if (FD_ISSET(fd_target_generator, &read_fds)) {
            ssize_t bytes_read = read(fd_target_generator, &targets_from_subscriber, sizeof(targets_from_subscriber));


            printf("Read %zd bytes from FIFO.\n", bytes_read);

            break;
            } 
            
            else {
            printf("No data available on FIFO. Retrying...\n");
            continue;
            }
        }

        int num_targets = targets_from_subscriber.num_targets; // the number of targets that the subscriber node is going to mention in the msg
        printf("Received %d targets.\n", num_targets);

        int targets[MAX_TARGETS][2] = {0};

        for (int i = 0; i < n_targets; i++) {
            targets[i][0] = targets_from_subscriber.targets[i][0];
            targets[i][1] = targets_from_subscriber.targets[i][1];

             printf("(%d, %d) ", targets[i][0], targets[i][1]);
        }



        ssize_t bytes_written = write(fd_target_generator_to_server, targets, sizeof(targets));
        if (bytes_written == -1) {
            perror("Error writing to FIFO");
            close(fd_target_generator_to_server);
            return 1;
        } else {
            printf("Successfully wrote %zd bytes to server FIFO.\n", bytes_written);
        }
        log_message(log_file, INFO, "TargetsGenerator sending targets.");

        while (!reset && !stop) {
            log_message(log_file, INFO, "TargetsGenerator running.");
            usleep(10000);
        }

        if (reset) {
            close(fd_target_generator_to_server);
            goto start;
        }

        if (stop) {
            usleep(1000000);
            close(fd_target_generator_to_server);
            log_message(log_file, INFO, "TargetsGenerator shutting down.");
            exit(0);
        }

        log_message(log_file, INFO, "TargetsGenerator shutting down.");
        close(fd_target_generator_to_server);
        return 0;
    }
}