

#include "server_functions.h"
#include"sig_handle.h"
#include"logger.h"



int main() {
    


    char* log_file = "../Logs/BlackBoardServer.log";

    log_message(log_file, INFO, "BlackBoardServer started successfully.");
    //registering the signals for Reset/continue/Pause
    signal(SIGUSR1, handle_pause_signal);
    signal(SIGUSR2, handle_reset_signal);
    signal(SIGINT, handle_stop_signal);


    
    int fifo_id=0;
    int fd_Keyboard_to_server = create_and_open_fifo("/tmp/keyboardManager_to_server_%d",0, O_RDONLY|O_NONBLOCK);

    
start:

    if (reset){

        log_message(log_file, INFO, "BlackBoardServer reset.");

        fifo_id++;
        reset=false;
        
        usleep(10000);

    }




    // Create FIFOs
    int fd_server_to_Dynamics = create_and_open_fifo("/tmp/server_to_DroneDynamics_%d",fifo_id, O_WRONLY);
    int fd_Dynamics_to_server = create_and_open_fifo("/tmp/DroneDynamics_to_server_%d",fifo_id, O_RDONLY|O_NONBLOCK);

    int fd_server_to_GameWindow = create_and_open_fifo("/tmp/server_to_GameWindow_%d",fifo_id, O_WRONLY);
    int fd_target_generator_to_server = create_and_open_fifo("/tmp/target_generator_to_server_%d",fifo_id, O_RDONLY | O_NONBLOCK);
    int fd_obstacle_generator_to_server = create_and_open_fifo("/tmp/obstacle_generator_to_server_%d",fifo_id, O_RDONLY | O_NONBLOCK);



    // setting up the necessary file descriptors for the select method
    fd_set read_fds;
    int fds[] = {fd_Dynamics_to_server, fd_Keyboard_to_server, fd_target_generator_to_server, fd_obstacle_generator_to_server};
    int max_rfd = get_max_fd(fds, 4);


    fd_set write_fds;
    int fds_w[] = {fd_server_to_GameWindow, fd_server_to_Dynamics};
    int max_wfd = get_max_fd(fds, 2);

    //getting the max fd number between  the read and write
    int max_fd = max_rfd>max_wfd? max_rfd:max_wfd;


    //for the select timeout
    struct timeval timeout = {0, 0};
    long last_frame_time = current_time_in_ms();




    int n_obstacles;
    int n_targets;

    // Retrieve the number of obstacles and targets from the JSON configuration file
    get_int_from_json("../Game_Config.json", "num_of_obstacles", &n_obstacles);
    get_int_from_json("../Game_Config.json", "num_of_targets", &n_targets); 


    ServerState state = initialize_server_state( n_obstacles,n_targets);   

    KeyboardInput prev_input={0};
    KeyboardInput input={0};




    while (1) {



    // Retrieve the number of obstacles and targets from the JSON configuration file
    get_int_from_json("../Game_Config.json", "num_of_obstacles", &n_obstacles);
    get_int_from_json("../Game_Config.json", "num_of_targets", &n_targets);

    state.num_obstacles = n_obstacles;
    state.num_targets = n_targets;
    



    int fps_value;

    // Retrieve the FPS value
    get_int_from_json("../Game_Config.json", "FPS", &fps_value);


        bool new_obstacle_arrived = false;




        if (is_paused) {

            usleep(100000); // Sleep while paused to reduce CPU usage
            
            log_message(log_file, INFO, "BlackBoardServer paused.");
            
            continue;
        }

        log_message(log_file, INFO, "BlackBoardServer running.");



        long current_time = current_time_in_ms();
        if (current_time - last_frame_time < 1000 / fps_value) {
            //printf("sleeping for sometime\n");
            fflush(stdout);
            usleep(1000); // Sleep for 1ms if we're ahead of the frame rate
            continue;
        }

        last_frame_time = current_time;


        FD_ZERO(&read_fds);
        FD_SET(fd_Dynamics_to_server, &read_fds);
        FD_SET(fd_Keyboard_to_server, &read_fds);
        FD_SET(fd_target_generator_to_server, &read_fds);
        FD_SET(fd_obstacle_generator_to_server, &read_fds);

        FD_ZERO(&write_fds);
        FD_SET(fd_server_to_GameWindow, &write_fds);
        FD_SET(fd_server_to_Dynamics, &write_fds);

        int activity = select(max_fd + 1, &read_fds, &write_fds, NULL, &timeout);



        if (activity > 0) {
        
            // Handle input from KeyboardManager
            if (FD_ISSET(fd_Keyboard_to_server, &read_fds)) {
                
                ssize_t bytes_read = read(fd_Keyboard_to_server, &input, sizeof(KeyboardInput));
                
                if (bytes_read == sizeof(KeyboardInput)) {
                    if (input.quit) {
                        printf("Quit signal received. Shutting down.\n");
                        break;
                    }

                if (memcmp(&input, &prev_input, sizeof(KeyboardInput)) ) {
                    state.input_x_force = input.force_x;
                    state.input_y_force = input.force_y;
                    //printf("Received from Keyboard: Force X = %d, Force Y = %d\n", input.force_x, input.force_y);

                        }
                    
                }
            }

        // Handle input from Target Generator
        if (FD_ISSET(fd_target_generator_to_server, &read_fds)) {
            int new_targets[n_targets][2];
            ssize_t bytes_read = read(fd_target_generator_to_server, new_targets, sizeof(new_targets));
            if (bytes_read == sizeof(new_targets)) {
                // Copy data into state struct
                memcpy(state.targets, new_targets, sizeof(new_targets));
                state.num_targets = n_targets;

            for (int i = 0; i < n_targets; i++) {
                printf("Target %d: (%d, %d)\n", i, state.targets[i][0], state.targets[i][1]);
            }
            }
        }
        
        // Handle input from Obstacle Generator
        if (FD_ISSET(fd_obstacle_generator_to_server, &read_fds)) {
            int new_obstacles[n_obstacles][2];
            ssize_t bytes_read = read(fd_obstacle_generator_to_server, new_obstacles, sizeof(new_obstacles));
            if (bytes_read == sizeof(new_obstacles)) {
                // Copy data into state struct
                memcpy(state.obstacles, new_obstacles, sizeof(new_obstacles));
                state.num_obstacles = n_obstacles;
                new_obstacle_arrived = true;
                // Print received obstacles
                printf("Received %d obstacles:\n", state.num_obstacles);
                for (int i = 0; i < n_obstacles; i++) {
                    printf("Obstacle %d: (%d, %d)\n", i, state.obstacles[i][0], state.obstacles[i][1]);
                }
            } else {
                fprintf(stderr, "Failed to read the correct number of bytes from obstacle generator.\n");
            }
}


        // Send updated state to GameWindow
        if (FD_ISSET(fd_server_to_GameWindow, &write_fds)) {

                write(fd_server_to_GameWindow, &state, sizeof(ServerState));

            }


        //write new forces to the drone dynamicss
        if (FD_ISSET(fd_server_to_Dynamics, &write_fds)) {
                

            if (prev_input.force_x!= input.force_x || prev_input.force_y!= input.force_y || new_obstacle_arrived)
            {

            printf("i am sending to the dynamics now %d %d \n",state.input_x_force,state.input_y_force);

            if(!reset){
                
                write(fd_server_to_Dynamics, &state, sizeof(ServerState));

            }

            new_obstacle_arrived = false;
            prev_input=input;
            }

        }

        // Handle input from DroneDynamics
            if (FD_ISSET(fd_Dynamics_to_server, &read_fds)) {
                ssize_t bytes_read = read(fd_Dynamics_to_server, &state, sizeof(ServerState));
                if (bytes_read == sizeof(ServerState)) {
                    // Enforce geofence boundaries

                    printf("Updated state received from DroneDynamics %f\n ",state.drone_x);
                }


            }   


    }
    
    if (reset){
    

        close(fd_Dynamics_to_server);
        close(fd_server_to_Dynamics);
        close(fd_server_to_GameWindow);
        close(fd_target_generator_to_server);
        close(fd_obstacle_generator_to_server);

        goto start;

    }

    }


    log_message(log_file, INFO, "BlackBoardServer shutting down.");



    // Close pipes
    close(fd_Dynamics_to_server);
    close(fd_server_to_Dynamics);
    close(fd_server_to_GameWindow);
    close(fd_Keyboard_to_server);
    close(fd_target_generator_to_server);
    close(fd_obstacle_generator_to_server);

    return 0;
}
