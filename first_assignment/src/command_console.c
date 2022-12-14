#include "./../include/command_utilities.h"
#include <unistd.h>
#include <fcntl.h>

int fd;


int Vx_m1; // inizialize the file descriptor of the pipe Vx
int Vz_m2; // inizialize the file descriptor of the pipe Vz
char *Vx = "/named_pipes/Vx"; // initialize the pipe Vx pathname
char *Vz = "/named_pipes/Vz"; // initialize the pipe Vx pathname

int V_msg[] = {0, 1, 2};
int (*V_msg_pp)[3] = &V_msg;

int main(int argc, char const *argv[]){
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize User Interface 
    init_console_ui();
    
    // open the pipes:
    Vx_m1 = open(Vx, O_WRONLY); // open the pipe to write on it
    if(Vx_m1 < 0){
        perror("error while opening the pipe Vx from cmd"); // checking errors
    } 
    Vz_m2 = open(Vz, O_WRONLY); // open the pipe to write on it
    if(Vz_m2 < 0){
        perror("error while opening the pipe Vz from cmd"); // checking errors
    } 


    // Infinite loop
    while(TRUE)
	{	
        // Get mouse/resize commands in non-blocking mode...
        int cmd = getch();

        // If user resizes screen, re-draw UI
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }
        // Else if mouse has been pressed
        else if(cmd == KEY_MOUSE) {

            // Check which button has been pressed...
            if(getmouse(&event) == OK) {

                // Vx-- button pressed
                if(check_button_pressed(vx_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Decreased");
                    refresh();

                    if(write(Vx_m1, V_msg_pp, 1) != 1) { // writing the number 0 on the pipe (0= Vx--)
                        perror("error tring to write on the Vx pipe from cmd (Vx--)"); // checking errors
                    }
                    sleep(1); // wait for one second
                }

                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                    refresh();

                    if(write(Vx_m1, (V_msg_pp + 2), 1) != 1) { // writing the number 2 on the pipe (2= Vx++)
                        perror("error tring to write on the Vx pipe from cmd (Vx++)"); // checking errors
                    }
                    sleep(1); // wait for one second
                }

                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                    refresh();

                    if(write(Vx_m1, (V_msg_pp + 1), 1) != 1) { // writing the number 1 on the pipe (1= Vx=0)
                        perror("error tring to write on the Vx pipe from cmd (Vx=0)"); // checking errors
                    }
                    sleep(1); // wait for one second
                }

                // Vz-- button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                    refresh();
                    if(write(Vz_m2, V_msg_pp, 1) != 1) { // writing the number 0 on the pipe (0= Vz--)
                        perror("error tring to write on the Vz pipe from cmd (Vz--)"); // checking errors
                    }
                    sleep(1); // wait for one second
                }

                // Vz++ button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                    refresh();
                    if(write(Vz_m2, (V_msg_pp + 2), 1) != 1) { // writing the number 2 on the pipe (2= Vz++)
                        perror("error tring to write on the Vz pipe from cmd (Vz++)"); // checking errors
                    }
                    sleep(1); // wait for one second
                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
                    refresh();
                    if(write(Vz_m2, (V_msg_pp + 1), 1) != 1) { // writing the number 1 on the pipe (1= Vz=0)
                        perror("error tring to write on the Vz pipe from cmd (Vz=0)"); // checking errors
                    }
                    sleep(1); // wait for one second
                }
            }
        }

        refresh();
	}

    // Terminate
    endwin();
    return 0;
}
