#include "Dynamics_functions.h"
#include "sig_handle.h"
#include"logger.h"


int MAX_X=100;
int MAX_Y=30;
int fps_value;


int main() {


    char *log_file = "../Logs/DroneDynamicsManager.log";
    signal(SIGUSR1, handle_pause_signal);
    signal(SIGUSR2, handle_reset_signal);
    signal(SIGINT, handle_stop_signal);
    int fifo_id=0;



    get_int_from_json("../Game_Config.json", "MAX_X", &MAX_X);
    get_int_from_json("../Game_Config.json", "MAX_Y", &MAX_Y);



    log_message(log_file, INFO, "DroneDynamicsManager started successfully.");

start:


    if(reset){


        get_int_from_json("../Game_Config.json", "MAX_X", &MAX_X);
        get_int_from_json("../Game_Config.json", "MAX_Y", &MAX_Y);
        get_int_from_json("../Game_Config.json", "FPS", &fps_value);
        log_message(log_file, INFO, "DroneDynamicsManager reset.");
        fifo_id++;
        reset=false;
        usleep(10000);
    }





    int fd_server_to_Dynamics = create_and_open_fifo("/tmp/server_to_DroneDynamics_%d",fifo_id, O_RDONLY|O_NONBLOCK);
    int fd_Dynamics_to_server = create_and_open_fifo("/tmp/DroneDynamics_to_server_%d",fifo_id, O_WRONLY);




    ServerState state = {0};
    Vector_2D velocity = {0, 0};
    Vector_2D acceleration = {0, 0};

    fd_set read_fds;
    struct timeval timeout = {0, 0};

  
    ServerState prev_state = initialize_server_state();








    
    while (!stop) {



    

    // Retrieve the FPS value
    get_int_from_json("../Game_Config.json", "FPS", &fps_value);

        if (is_paused) {

            log_message(log_file, INFO, "DroneDynamicsManager paused.");
            usleep(100000); // Sleep while paused to reduce CPU usage
            continue;
        }


        FD_ZERO(&read_fds);
        FD_SET(fd_server_to_Dynamics, &read_fds);

        
        
        int activity = select(fd_server_to_Dynamics + 1, &read_fds, NULL, NULL, &timeout);

        if(activity>0){
        ssize_t bytes = read(fd_server_to_Dynamics, &state, sizeof(ServerState));
        }
        

        else 
        {
            //printf("here ");
            //fflush(stdout);
   
            //prev_state.input_x_force=0;
            //prev_state.input_y_force=0;

            //state=prev_state;
        }

        printf("%f\n",state.drone_x);
        

        Vector_2D repulsion = compute_repulsion_forces(state.input_x_force,state.input_y_force,state.drone_x, state.drone_y, state.num_obstacles, state.obstacles);
        Vector_2D viscosity = compute_viscosity_force(state.velocity_x, state.velocity_y);


        printf("force input %d %d \n",state.input_x_force,state.input_y_force);
        printf("resultant force %f %f \n",(state.input_x_force + repulsion.x + viscosity.x) ,(state.input_y_force + repulsion.y + viscosity.y) );

        printf("repulsion %f %f\n",repulsion.x,repulsion.y);
        printf("vis %f %f\n",viscosity.x,viscosity.y);

        acceleration.x = (state.input_x_force + repulsion.x + viscosity.x) / DRONE_MASS;
        acceleration.y = (state.input_y_force + repulsion.y + viscosity.y) / DRONE_MASS;
        printf("acceleration %f %f\n",acceleration.x,acceleration.y);

        velocity.x = state.velocity_x + acceleration.x * TIME_STEP;
        velocity.y = state.velocity_y + acceleration.y * TIME_STEP;
        printf("velocity %f %f\n",velocity.x,velocity.y);

        state.drone_x =state.drone_x + velocity.x * TIME_STEP;
        state.drone_y =state.drone_y + velocity.y * TIME_STEP;

        printf("pos %f %f\n",state.drone_x,state.drone_y);


        state.velocity_x = velocity.x;
        state.velocity_y = velocity.y;
        


        enforce_geofence(&state,MAX_X,MAX_Y);




        write(fd_Dynamics_to_server, &state, sizeof(ServerState));


        prev_state=state;
        log_message(log_file, INFO, "DroneDynamicsManager running.");

        if (reset){

            close(fd_Dynamics_to_server);
            close(fd_server_to_Dynamics);
            goto start;
            
        }

        usleep(1000000 / fps_value);
    }

    close(fd_Dynamics_to_server);
    close(fd_server_to_Dynamics);
    return 0;
}
