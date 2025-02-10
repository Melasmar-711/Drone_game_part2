#include"Window.h"
#include"sig_handle.h"
#include"logger.h"
#include<sys/select.h>



int MAX_X=100 ;
int MAX_Y=30 ;


int main() {
    
    char*   log_file = "../Logs/GameWindow.log";

    get_int_from_json("../Game_Config.json","MAX_X",&MAX_X);
    get_int_from_json("../Game_Config.json","MAX_Y",&MAX_Y);

    int n_obstacles;
    int n_targets;
    get_int_from_json("../Game_Config.json","num_of_obstacles",&n_obstacles);
    get_int_from_json("../Game_Config.json","num_of_targets",&n_targets);


    signal(SIGUSR1, handle_pause_signal);
    signal(SIGUSR2, handle_reset_signal);
    signal(SIGINT, handle_stop_signal);
    int fifo_id=0;
    fd_set read_fds;
    struct timeval timeout = {0, 0};

    log_message(log_file, INFO, "GameWindow started successfully.");

start:


    if(reset){



        just_got_reset=true;

        clear();
        fifo_id++;
        reset=false;
        log_message(log_file, INFO, "GameWindow reset.");
        usleep(10000);
        
        get_int_from_json("../Game_Config.json","MAX_X",&MAX_X);
        get_int_from_json("../Game_Config.json","MAX_Y",&MAX_Y);
        get_int_from_json("../Game_Config.json","num_of_obstacles",&n_obstacles);
        get_int_from_json("../Game_Config.json","num_of_targets",&n_targets);

    }


    int fd_server_to_GameWindow = create_and_open_fifo("/tmp/server_to_GameWindow_%d",fifo_id, O_RDONLY|O_NONBLOCK);





    // Server state
    ServerState state=initialize_server_state(n_obstacles,n_targets);


    int target_active_flags[state.num_obstacles];
    memset(target_active_flags, 0, sizeof(target_active_flags)); // Set all bytes of arr to 0



    ServerState prev_state=initialize_server_state(n_obstacles,n_targets);
    prev_state.drone_x=0;
    prev_state.drone_y=0;
    prev_state.velocity_x=0;
    prev_state.velocity_y=0;

    init_ncurses();
    draw_borders(MAX_X, MAX_Y);

    refresh();


    while (!stop) {




        if (is_paused) {
            log_message(log_file, INFO, "GameWindow paused.");
            usleep(100000); // Sleep while paused to reduce CPU usage
            continue;
        }



        ssize_t bytes_read = read(fd_server_to_GameWindow, &state, sizeof(prev_state));

        if(bytes_read!=sizeof(ServerState)){
            log_message(log_file, ERROR, "Failed to read the correct number of bytes from server.");
            state=prev_state;
            
            continue;
        }

        
    
        draw_simulation(&prev_state,&state,target_active_flags);

        refresh();

        prev_state=state; 
        log_message(log_file, INFO, "GameWindow running.");


        if(reset){
            close(fd_server_to_GameWindow);

            goto start;
        }


        usleep(DELAY);



    }






    endwin();
    close(fd_server_to_GameWindow);
    return 0;
}




