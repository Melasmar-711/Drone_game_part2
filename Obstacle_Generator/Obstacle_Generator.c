#include "Generator_functions.h"
#include "sig_handle.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mode>\n", argv[0]);
        return 1;
    }

    char *mode = argv[1];
    printf("Mode: %s\n", mode); // Debug print

    if (strcmp(mode, "publisher") != 0 && strcmp(mode, "subscriber") != 0) {
        fprintf(stderr, "Invalid mode. Use 'publisher' or 'subscriber'.\n");
        while(1); // Ensures watchdog detects an issue
    }

    char *log_file = "../Logs/ObstacleGenerator.log";
    signal(SIGUSR1, handle_pause_signal);
    signal(SIGUSR2, handle_reset_signal);
    signal(SIGINT, handle_stop_signal);
    
    log_message(log_file, INFO, "ObstacleGenerator started successfully.");
    printf("ObstacleGenerator started successfully.\n"); // Debug print

    int fifo_id = 0;
    int fps_value, MAX_X, MAX_Y, n_obstacles;
    
    get_int_from_json("../Game_Config.json", "MAX_X", &MAX_X);
    get_int_from_json("../Game_Config.json", "MAX_Y", &MAX_Y);
    get_int_from_json("../Game_Config.json", "FPS", &fps_value);
    get_int_from_json("../Game_Config.json", "num_of_obstacles", &n_obstacles);

    printf("Configuration: MAX_X=%d, MAX_Y=%d, FPS=%d, num_of_obstacles=%d\n", MAX_X, MAX_Y, fps_value, n_obstacles); // Debug print

    int fd_obstacle_generator;
    
    if (strcmp(mode, "publisher") == 0) {
        const char *fifo_path = "/tmp/obstacle_generator_fifo_pub_%d";
        fd_obstacle_generator = create_and_open_fifo(fifo_path, fifo_id, O_WRONLY);
        printf("Publisher mode: FIFO created and opened for writing.\n"); // Debug print
    } else {
        const char *fifo_path = "/tmp/obstacles_generator_fifo_sub_%d";
        fd_obstacle_generator = create_and_open_fifo(fifo_path, fifo_id, O_RDONLY);
        printf("Subscriber mode: FIFO created and opened for reading.\n"); // Debug print
    }

start:
    if (reset) {
        log_message(log_file, INFO, "ObstacleGenerator reset.");
        printf("Resetting ObstacleGenerator.\n"); // Debug print
        fifo_id++;
        reset = false;
        //print reset value to make sure it got set to false
        printf("Reset value: %d\n", reset); // Debug print
        
        get_int_from_json("../Game_Config.json", "MAX_X", &MAX_X);
        get_int_from_json("../Game_Config.json", "MAX_Y", &MAX_Y);
        get_int_from_json("../Game_Config.json", "num_of_obstacles", &n_obstacles);
        get_int_from_json("../Game_Config.json", "FPS", &fps_value);
        
        usleep(100000);
    }

    srand(time(NULL) + 2);

    if (strcmp(mode, "publisher") == 0) {
        while (1) {
            int num_obstacles = n_obstacles;
            int obstacles[num_obstacles][2];

            for (int i = 0; i < num_obstacles; i++) {
                obstacles[i][0] = (rand() % ((MAX_X - 1) / 2)) * 2 + 1;
                obstacles[i][1] = (rand() % ((MAX_Y - 1) / 2)) * 2 + 1;
            }

            Obstacle_gen obstacles_to_publish = {0};
            obstacles_to_publish.num_obstacles = num_obstacles;
            memcpy(obstacles_to_publish.obstacles, obstacles, sizeof(obstacles));
            
            write(fd_obstacle_generator, &obstacles_to_publish, sizeof(obstacles_to_publish));
            log_message(log_file, INFO, "ObstacleGenerator publishing obstacles.");
            printf("Generated and sent %d obstacles.\n", num_obstacles); // Debug print

            usleep(1000000);
            if (reset) goto start;
            if (stop) break;
        }
    } else if (strcmp(mode, "subscriber") == 0) {
        int fd_obstacle_generator_to_server = create_and_open_fifo("/tmp/obstacle_generator_to_server_%d", fifo_id, O_WRONLY);
        Obstacle_gen obstacles_from_subscriber = {0};
        
        fd_set read_fds;
        struct timeval timeout;
        FD_ZERO(&read_fds);
        FD_SET(fd_obstacle_generator, &read_fds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        

        while(1){
        while (1) {
            log_message(log_file, INFO, "ObstacleGenerator running in subscriber mode.");
            printf("Running in subscriber mode.\n"); // Debug print

            printf("Reset value: %d\n", reset); // Debug print
            if (reset) {
                
                close(fd_obstacle_generator_to_server);
                goto start;
            }
            if (stop) break;
            
            int ret = select(fd_obstacle_generator + 1, &read_fds, NULL, NULL, &timeout);
            if (FD_ISSET(fd_obstacle_generator, &read_fds)) {
                read(fd_obstacle_generator, &obstacles_from_subscriber, sizeof(obstacles_from_subscriber));

                //for loop to print what we receive in pairs
                for (int i = 0; i < obstacles_from_subscriber.num_obstacles; i++) {
                    printf("(%d, %d) ", obstacles_from_subscriber.obstacles[i][0], obstacles_from_subscriber.obstacles[i][1]);
                }
                break;
            }
        }
        
        int num_obstacles = obstacles_from_subscriber.num_obstacles;
        printf("Received %d obstacles.\n", num_obstacles); // Debug print
        int obstacles[MAX_OBSTACLES][2];
        memset(obstacles, 0, sizeof(obstacles));
        for (int i = 0; i < num_obstacles; i++) {
            obstacles[i][0] = obstacles_from_subscriber.obstacles[i][0];
            obstacles[i][1] = obstacles_from_subscriber.obstacles[i][1];
        }
        write(fd_obstacle_generator_to_server, obstacles, sizeof(obstacles));
        log_message(log_file, INFO, "ObstacleGenerator sending obstacles.");
        printf("Sending obstacles to server.\n"); // Debug print
        
        for(int i=0;i<20;i++){

            if(reset){
                close(fd_obstacle_generator_to_server);
                goto start;
            } 
            else{  
                log_message(log_file, INFO, "ObstacleGenerator running.");
                usleep(500000);
            }

        }
        
        }
    }


    log_message(log_file, INFO, "ObstacleGenerator shutting down.");
    printf("Shutting down ObstacleGenerator.\n"); // Debug print
    close(fd_obstacle_generator);
    return 0;
}
