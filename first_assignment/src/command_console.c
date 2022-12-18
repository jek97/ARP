#include "./../include/command_utilities.h"
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

int logger(char * log_pathname, char log_msg[]) {
  double c = (double) (clock() / CLOCKS_PER_SEC);
  char log_msg_arr[strlen(log_msg)+11];
  if ((sprintf(log_msg_arr, " %s,%.2E;", log_msg, c)) < 0){
    perror("error in logger sprintf");
    return -1;
  }
  int log_fd; // declare the log file descriptor
  if ((log_fd = open(log_pathname,  O_CREAT | O_APPEND | O_WRONLY, 0644)) < 0){ // open the log file to write on it
    perror(("error opening the log file %s", log_pathname)); // checking errors
    return -1;
  }
  if(write(log_fd, log_msg_arr, sizeof(log_msg_arr)) != sizeof(log_msg_arr)) { // writing the log message on the log file
      perror("error tring to write the log message in the log file"); // checking errors
      return -1;
  }
  close(log_fd);
  return 1;
}

int main(int argc, char const *argv[]){

    int Vx_m1; // inizialize the file descriptor of the pipe Vx
    int Vz_m2; // inizialize the file descriptor of the pipe Vz
    char *Vx = "./bin/named_pipes/Vx"; // initialize the pipe Vx pathname
    char *Vz = "./bin/named_pipes/Vz"; // initialize the pipe Vx pathname

    int V_msg[] = {0, 1, 2};
    int (*V_msg_pp)[3] = &V_msg;

    char * log_pn_command = "./bin/log_files/command.txt"; // initialize the log file path name
    remove(log_pn_command);
    logger(log_pn_command, "log legend: 0001=opened the pipes  0010= Vx--  0011 = Vx++  0100= Vx=0  0101= Vz--  0110= Vz++  0111= Vz=0.    the log number with an e in front means the relative operation failed");
    
    // open the pipes:
    Vx_m1 = open(Vx, O_WRONLY); // open the pipe to write on it
    if(Vx_m1 < 0){
        perror("error while opening the pipe Vx from cmd"); // checking errors
    } 

    Vz_m2 = open(Vz, O_WRONLY); // open the pipe to write on it
    if(Vz_m2 < 0){
        perror("error while opening the pipe Vz from cmd"); // checking errors
    }

    if (Vx_m1 > 0 && Vz_m2 > 0) {
        logger(log_pn_command, "0001"); // write a log message
    }
    else {
        logger(log_pn_command, "e0001"); // write a error log message
    }

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize User Interface 
    init_console_ui();

    // Infinite loop
    while(TRUE){

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
                        logger(log_pn_command, "e0010"); // write a error log message
                    }
                    else {
                        logger(log_pn_command, "0010"); // write a log message
                    }
                    sleep(1); // wait for one second
                }

                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                    refresh();

                    if(write(Vx_m1, (V_msg_pp + 2), 1) != 1) { // writing the number 2 on the pipe (2= Vx++)
                        perror("error tring to write on the Vx pipe from cmd (Vx++)"); // checking errors
                        logger(log_pn_command, "e0011"); // write a error log message
                    }
                    else {
                        logger(log_pn_command, "0011"); // write a log message
                    }
                    sleep(1); // wait for one second
                }

                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                    refresh();

                    if(write(Vx_m1, (V_msg_pp + 1), 1) != 1) { // writing the number 1 on the pipe (1= Vx=0)
                        perror("error tring to write on the Vx pipe from cmd (Vx=0)"); // checking errors
                        logger(log_pn_command, "e0100"); // write a error log message                        
                    }
                    else {
                        logger(log_pn_command, "0100"); // write a log message
                    }
                    sleep(1); // wait for one second
                }

                // Vz-- button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                    refresh();
                    if(write(Vz_m2, V_msg_pp, 1) != 1) { // writing the number 0 on the pipe (0= Vz--)
                        perror("error tring to write on the Vz pipe from cmd (Vz--)"); // checking errors
                        logger(log_pn_command, "e0101"); // write a error log message
                    }
                    else {
                        logger(log_pn_command, "0101"); // write a log message
                    }
                    sleep(1); // wait for one second
                }

                // Vz++ button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                    refresh();
                    if(write(Vz_m2, (V_msg_pp + 2), 1) != 1) { // writing the number 2 on the pipe (2= Vz++)
                        perror("error tring to write on the Vz pipe from cmd (Vz++)"); // checking errors
                        logger(log_pn_command, "e0110"); // write a error log message
                    }
                    else {
                        logger(log_pn_command, "0110"); // write a log message
                    }
                    sleep(1); // wait for one second
                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
                    refresh();
                    if(write(Vz_m2, (V_msg_pp + 1), 1) != 1) { // writing the number 1 on the pipe (1= Vz=0)
                        perror("error tring to write on the Vz pipe from cmd (Vz=0)"); // checking errors
                        logger(log_pn_command, "e0111"); // write a error log message
                    }
                    else {
                        logger(log_pn_command, "0111"); // write a log message
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
