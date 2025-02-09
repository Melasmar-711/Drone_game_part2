#include"KeyBoard.h"








void init_ncurses() {
    initscr();
    noecho();
    curs_set(FALSE);
    nodelay(stdscr, TRUE);
    //timeout(100);
    keypad(stdscr, TRUE);
}


void process_input(KeyboardInput *input) {
    int ch = getch(); 

    switch (ch) {
        case 'w': input->force_y-=1; break;
        case 's': input->force_x = 0; input->force_y = 0; break;
        case 'a': input->force_x-=1; break;
        case 'd': input->force_x+=1; break;
        case 'x': input->force_y+=1; break;
        case 'q': input->force_x -= 1; input->force_y-=1; break;
        case 'e': input->force_x += 1; input->force_y -=1; break;
        case 'z': input->force_x-=1; input->force_y += 1; break;
        case 'c': input->force_x+= 1; input->force_y+= 1; break;
        case 'o': input->quit = stop; break; // Quit
        case 'p': input->quit=Pause_or_Continue;break;
        case 'r':input->quit=Re_set;break;

        //case ERR :input->force_x = 0; input->force_y = 0; break;
        default: break;  
    }
}



void draw_keyboard_layout(KeyboardInput *input) {

    mvprintw(5, 10, "Keyboard Layout:");
    mvprintw(7, 10, "  q | w | e  ");
    mvprintw(8, 10, "  -----------  ");
    mvprintw(9, 10, "  a | s | d  ");
    mvprintw(10, 10, "  -----------  ");
    mvprintw(11, 10, "  z | x | c  ");

    mvprintw(13, 10, "Force X: %d", input->force_x);
    mvprintw(14, 10, "Force Y: %d", input->force_y);

    mvprintw(16, 10, "Controls:");
    mvprintw(17, 10, "'w': Move Up, 'a': Move Left, 's': Stop, 'd': Move Right");
    mvprintw(18, 10, "'q': Up-Left, 'e': Up-Right, 'z': Down-Left, 'c': Down-Right");
    mvprintw(19, 10, "'o': Quit");


}
