#include "Generator_functions.h"
#include "sig_handle.h"
#include "logger.h"

int main(int argc, char *argv[]) {



    if (argc >= 2) {
        fprintf(stderr, "Usage: %s <mode>\n", argv[1]);
        //return 1;
    }

    char *mode = argv[1];
   if (strcmp(mode, "publisher") != 0 && strcmp(mode, "subscriber") != 0) {
        fprintf(stderr, "Invalid mode. Use 'publisher' or 'subscriber'.\n");
        while(1);//this will make it not log so the watchdog will know
                //   something is wrong and kill it along with the rest of the processes
    }




    
    int fd_target_generator;
    if (strcmp(mode, "publisher") == 0) {
        // Create a named FIFO


        // Fork and exec publisher node and pass the FIFO to it
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // Child process
            execlp("../build/Targets_publisher/Targets", "Targets", "publisher", NULL);
            perror("execlp"); // If execlp fails
            exit(EXIT_FAILURE);
        } 
        
        else {

            


            const char *fifo_path = "/tmp/target_generator_fifo";
            int fd_target_generator = create_and_open_fifo(fifo_path, 0, O_WRONLY);

            
            if (fd_target_generator == -1) {
                perror("create_and_open_fifo");
                exit(EXIT_FAILURE);
            }

        }

    } 
    
    else if (strcmp(mode, "subscriber") == 0) {
        // Create a named FIFO
        const char *fifo_path = "/tmp/target_generator_fifo";
        int fd_target_generator = create_and_open_fifo(fifo_path, 0, O_RDONLY);
        if (fd_target_generator == -1) {
            perror("create_and_open_fifo");
            exit(EXIT_FAILURE);
        }

        // Fork and exec subscriber node and pass the FIFO to it
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // Child process
            execlp("../build/Targets_publisher/Targets", "Targets", "subscriber", NULL);
            perror("execlp"); // If execlp fails
            exit(EXIT_FAILURE);
        } 
        
        else {
            // Parent process
            printf("Opening FIFO for reading\n");
            fd_target_generator = open(fifo_path, O_RDONLY);
            if (fd_target_generator == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }
        }
    }






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

    int n_targets;
    get_int_from_json("../Game_Config.json", "num_of_targets", &n_targets);


start:
    if (reset) {
        log_message(log_file, INFO, "TargetsGenerator reset.");
        fifo_id++;
        reset = false;
        get_int_from_json("../Game_Config.json", "MAX_X", &MAX_X);
        get_int_from_json("../Game_Config.json", "MAX_Y", &MAX_Y);
        get_int_from_json("../Game_Config.json", "FPS", &fps_value);
        get_int_from_json("../Game_Config.json", "num_of_targets", &n_targets);

        usleep(10000);
    }

    // Seed random number generator
    srand(time(NULL) + 1);

    // Create FIFO
    int fd_target_generator_to_server = create_and_open_fifo("/tmp/target_generator_to_server_%d", fifo_id, O_WRONLY);



    //if in the publisher mode then the numebr of targets will be  the same as the json file
    if (strcmp(mode, "publisher") == 0){

        int num_targets = n_targets;
        int targets[n_targets][2];


        for (int i = 0; i < num_targets; i++) {

            // Generate random targets within specified boundaries, ensuring even positions

            targets[i][0] = (rand() % ((MAX_X - 2) / 2)) * 2 + 2;  // X coordinate (even)
            targets[i][1] = (rand() % ((MAX_Y - 2) / 2)) * 2 + 2;  // Y coordinate (even)

        }



        //send the targets to the publisehr node through the unnamed pipe
        Targets_gen targets_to_publish;
        targets_to_publish.num_targets = num_targets;
        memcpy(targets_to_publish.targets, targets, sizeof(targets));

        ssize_t bytes_written = write(fd_target_generator, &targets_to_publish, sizeof(targets_to_publish));
    }

    else if (strcmp(mode, "subscriber") == 0){

        //receive from the subscriber node the from the unnamed pipe the stuct that has the targets and the number of targets

        Targets_gen targets_from_subscriber;
        ssize_t bytes_read = read(fd_target_generator, &targets_from_subscriber, sizeof(targets_from_subscriber));  
        
        int num_targets = targets_from_subscriber.num_targets;// the number of targets that the subscriber node is going to mention in the msg


        //write to the json file
        write_int_to_json("../Game_Config.json", "num_of_targets", num_targets);


        int targets[num_targets][2];

        // Send the targets array to the server
        ssize_t bytes_written = write(fd_target_generator_to_server, targets, sizeof(targets));
        if (bytes_written == -1) {
            perror("Error writing to FIFO");
            close(fd_target_generator_to_server);
            return 1;
        }

        printf("Generated and sent %d targets.\n", num_targets);


    }



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