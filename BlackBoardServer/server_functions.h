#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H




#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <sys/time.h>
#include"shared.h"
#include <cjson/cJSON.h>






typedef struct {
    int force_x;
    int force_y;
    int quit;
} KeyboardInput;



long current_time_in_ms();
int get_max_fd(int fds[], int num_fds);






#endif // SERVER_FUNCTIONS_H

