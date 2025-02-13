#ifndef GENERATOR_FUNCTIONS_H
#define GENERATOR_FUNCTIONS_H






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
#include "shared.h"





// Data Structures
typedef struct {
    int x;
    int y;
} Target;


typedef struct {
    int num_targets;
    int targets[MAX_TARGETS][2];
} Targets_gen;


#endif // GENERATOR_FUNCTIONS_H

