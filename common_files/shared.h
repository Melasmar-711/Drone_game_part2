#ifndef SHARED
#define SHARED

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>






#define MAX_OBSTACLES 100
#define MAX_TARGETS 100
//#define MAX_X 100
//#define MAX_Y 30
#define FRAME_RATE 30




typedef struct {
    float drone_x;
    float drone_y;
    int input_x_force;
    int input_y_force;
    float resultant_force_x;
    float resultant_force_y;
    float velocity_x;
    float velocity_y;
    int num_obstacles;
    int obstacles[MAX_OBSTACLES][2];
    int num_targets;
    int targets[MAX_TARGETS][2];
} ServerState;





int create_and_open_fifo(const char *t, int identifier, int flags);
void unlink_fifo(const char *t, int identifier);
ServerState initialize_server_state(int n_obstacles, int n_targets) ;
pid_t get_pidd(const char *program_name) ;
int get_int_from_json(const char *filename, const char *key, int *value) ;
int write_int_to_json(const char *filename, const char *key, int value) ;


#endif