
#include"Window.h"



bool just_got_reset;

void init_ncurses() {
    initscr();
    noecho();
    curs_set(FALSE);
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);  // Drone
    init_pair(2, COLOR_GREEN, COLOR_BLACK); // Targets
    init_pair(3, COLOR_RED, COLOR_BLACK);   // Obstacles
}


void draw_borders(int MAX_X, int MAX_Y) {

    // Draw the static simulation boundary
    attron(COLOR_PAIR(3));
    for (int i = 0; i < MAX_X; i++) {
        mvprintw(0, i, "-");
        mvprintw(MAX_Y, i, "-");
    }

    for (int i = 0; i <= MAX_Y; i++) {
        mvprintw(i, 0, "|");
        mvprintw(i, MAX_X, "|");
    }

    refresh(); // Refresh to display the borders
    attroff(COLOR_PAIR(3)); // Stop using the obstacle color
}




void draw_simulation(ServerState *prev_state, ServerState *current_state,int *flags) {


     // Handle the drone position
    if (prev_state->drone_x != current_state->drone_x || prev_state->drone_y != current_state->drone_y) 
    {
        // Erase old drone position
        attron(COLOR_PAIR(1));
        mvprintw((int) prev_state->drone_y, (int)prev_state->drone_x, " ");
        // Draw new drone position
        mvprintw((int)current_state->drone_y, (int)current_state->drone_x, "+");

        attroff(COLOR_PAIR(1));
    }





    // Handle obstacles
    /*Handle whether the obstacles were moved or their number increased or decreased*/
    for (int i = 0; i < prev_state->num_obstacles; i++) 
    {

        attron(COLOR_PAIR(3));
            // Check if an obstacle moved

            
        if (prev_state->obstacles[i][0] != current_state->obstacles[i][0] ||
                prev_state->obstacles[i][1] != current_state->obstacles[i][1]) 
            {
                mvprintw(prev_state->obstacles[i][1], prev_state->obstacles[i][0]," "); // Erase old position
                //mvprintw(current_state->obstacles[i][1], current_state->obstacles[i][0], "O"); // Draw new position
            }
        
    }
    
    if(current_state->num_obstacles!=prev_state->num_obstacles){
        for(int i=0;i<current_state->num_obstacles;i++){
            mvprintw(current_state->obstacles[i][1], current_state->obstacles[i][0],"O");
        }
    }

    else{
        for (int i = 0; i < prev_state->num_obstacles; i++) {
            mvprintw(current_state->obstacles[i][1], current_state->obstacles[i][0], "O"); 
        }
    }

        attroff(COLOR_PAIR(3));
    




    
    // Handle targets
    /*Handle whether targets were collected , increasedd ,decreased*/
    for (int i = 0; i < prev_state->num_targets; i++) {


        static int prev_flags[MAX_TARGETS] = {0};
        static int score = 0;

        if(just_got_reset){
            score=0;
            memcpy(prev_flags, flags, sizeof(prev_flags));
            just_got_reset=false;
        }

        int dx = current_state->drone_x - current_state->targets[i][0];
        int dy = current_state->drone_y - current_state->targets[i][1];
        double distance = sqrt(dx * dx + dy * dy);


        if (distance<0.2 && prev_flags[i]!=1 )
        {
            flags[i]=1;  // this means it's taken now
            prev_flags[i]=1;
            score++;
        }
        attron(COLOR_PAIR(2));
        


        
        if (flags[i]==1 ) 
        {
            // Target removed
            mvprintw(prev_state->targets[i][1], prev_state->targets[i][0], " ");
            flags[i]=0;
        } 

 


 
        if (prev_state->targets[i][0] != current_state->targets[i][0] || prev_state->targets[i][1] != current_state->targets[i][1])      
        {


                mvprintw(prev_state->targets[i][1], prev_state->targets[i][0], " "); // Erase old position
                
                mvprintw(current_state->targets[i][1], current_state->targets[i][0], "T"); // Draw new position


        }



        mvprintw(35,40,"score : "); // the score

        mvprintw(35,48,"%d", score); // the score


        attroff(COLOR_PAIR(2));

    }

   if(current_state->num_targets!=prev_state->num_targets){
        for(int i=0;i<current_state->num_targets;i++){
            mvprintw(current_state->targets[i][1], current_state->targets[i][0],"T");
        }
    }



}

