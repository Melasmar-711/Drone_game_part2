
#include"KeyBoard.h"
#include"logger.h"



bool is_paused=false;

int main(int argc, char *argv[]) {


    char* log_file = "../Logs/KeyBoard.log";
    

    //check whether in publisher or subscriber mode
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mode>\n", argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    char Targets_exce_name[256];
    snprintf(Targets_exce_name, sizeof(Targets_exce_name), "Targets_Generator %s", mode);
    pid_t Targets_Generator=get_pidd(Targets_exce_name);

    




    //Collect PIDs of the Running the processes 
    
    pid_t server_pid=get_pidd("BlackBoardServer");
    pid_t GameWindow=get_pidd("GameWindow");
    pid_t DroneDynamicsManager=get_pidd("DroneDynamicsManager");

    pid_t Obstcales_Generator=get_pidd("Obstacle_Generator");



    


    KeyboardInput input = {0, 0, 0}, prev_input = {0, 0, 0}; // initialize the structure of inputs


    init_ncurses();

    log_message(log_file, INFO, "KeyboardManager started.");




    int fd_Keyboard_to_server = create_and_open_fifo("/tmp/keyboardManager_to_server_%d",0, O_WRONLY); 




    
    while (input.quit!=stop) {

        clear(); // Clear the screen

        draw_keyboard_layout(&input);

        //draw Targets_exce_name

                
        process_input(&input);  // Process user input and update the structure


        //sending pause or continue signal
        if (input.quit==Pause_or_Continue){   

            input.force_x=prev_input.force_x;
            input.force_y=prev_input.force_y;
            kill(server_pid, SIGUSR1);         
            kill(DroneDynamicsManager, SIGUSR1);
            usleep(10000);
            input.quit=0;
            continue;

        }


        //sending the stopping signal
        if (input.quit==stop){
        

            kill(server_pid, SIGINT);
            kill(DroneDynamicsManager, SIGINT);
            kill(Obstcales_Generator, SIGINT);
            //kill(Targets_Generator, SIGINT);
            kill(GameWindow, SIGINT);
            usleep(10000);
            exit(0);

        }

        //sending the reset signal
        if (input.quit==Re_set){

            input.force_x=0;
            input.force_y=0;

            kill(server_pid, SIGUSR2);
            kill(DroneDynamicsManager, SIGUSR2);
            kill(Obstcales_Generator, SIGUSR2);
            kill(Targets_Generator, SIGUSR2);
            kill(GameWindow, SIGUSR2);



            input.quit=0;
            
            usleep(10000);
            
        }

        

        write(fd_Keyboard_to_server, &input, sizeof(KeyboardInput));
        prev_input = input;

        refresh();

        log_message(log_file, INFO, "KeyboardManager running.");
        usleep(DELAY); // Control the frame rate
    }

    endwin();
    close(fd_Keyboard_to_server);
    log_message(log_file, INFO, "KeyboardManager shutting down.");
    
    return 0;

}




