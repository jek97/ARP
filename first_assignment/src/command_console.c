#include "./../include/command_utilities.h"
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

// declare some global variables:
int Vx_m1; // inizialize the file descriptor of the pipe Vx
int Vz_m2; // inizialize the file descriptor of the pipe Vz
const char *Vx = "./bin/named_pipes/Vx"; // initialize the pipe Vx pathname
const char *Vz = "./bin/named_pipes/Vz"; // initialize the pipe Vx pathname

int logger(const char * log_pathname, char log_msg[]) {
  int log_fd; // declare the log file descriptor
  char log_msg_arr[strlen(log_msg)+11]; // declare the message string
  float c = (float) (clock() / CLOCKS_PER_SEC); // evaluate the time from the program launch
  char * log_msg_arr_p = &log_msg_arr[0]; // initialize the pointer to the log_msg_arr array
  if ((sprintf(log_msg_arr, " %s,%.2E;", log_msg, c)) < 0){ // fulfill the array with the message
    perror("error in logger sprintf"); // checking errors
    return -1;
  }

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

void sig_handler(int signo) {
    if (signo == SIGTERM) {
        if (close(Vx_m1) < 0) { // close the pipe Vx
            perror("error closing the pipe Vx from command"); // checking errors
        }
        if (close(Vz_m2) < 0) { // close the pipe Vz
            perror("error closing the pipe Vz from command"); // checking errors
        }
        if (unlink(Vx) < 0) { // delete the file name from the system of the pipe Vx
            perror("error deleting the pipe Vx from command"); // checking errors
        }
        if (unlink(Vz) < 0) { // delete the file name from the system of the pipe Vz
            perror("error deleting the pipe Vz from command"); // checking errors
        }
        if (raise(SIGKILL) != 0) { // proces commit suicide
            perror("error suiciding the command"); // checking errors
        }
    }
}

int main(int argc, char const *argv[]){
    // declaring variables:

    int V_msg[] = {0, 1, 2}; // initialize the velocity messages array
    int * V_msg_p = &V_msg[0]; // initialize the pointer to the V_msg array

    ssize_t w_Vx_m1_1; // declaring the returned valeu of the function write to write the number 0 on the pipe Vx form cmd to motor1
    ssize_t w_Vx_m1_2; // declaring the returned valeu of the function write to write the number 1 on the pipe Vx form cmd to motor1
    ssize_t w_Vx_m1_3; // declaring the returned valeu of the function write to write the number 2 on the pipe Vx form cmd to motor1

    ssize_t w_Vz_m2_1; // declaring the returned valeu of the function write to write the number 0 on the pipe Vz form cmd to motor2
    ssize_t w_Vz_m2_2; // declaring the returned valeu of the function write to write the number 1 on the pipe Vz form cmd to motor2
    ssize_t w_Vz_m2_3; // declaring the returned valeu of the function write to write the number 2 on the pipe Vz form cmd to motor2

    const char * log_pn_command = "./bin/log_files/command.txt"; // initialize the log file path name

    if (signal(SIGTERM, sig_handler) == SIG_ERR) { // check if there is any closure signal
        perror("error receiving the closure signal from the master in command"); // checking errors
        logger(log_pn_command, "e1000"); // write a error log message
    }

    remove(log_pn_command); // remove the previous log file
    logger(log_pn_command, "log legend: 0001=opened the pipes  0010= Vx--  0011 = Vx++  0100= Vx=0  0101= Vz--  0110= Vz++  0111= Vz=0  1000= closure signal received.    the log number with an e in front means the relative operation failed");
    
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

    if (Vx_m1 > 0 && Vz_m2 > 0) {
        logger(log_pn_command, "0001"); // write a log message
    }
    else {
        logger(log_pn_command, "e0001"); // write a error log message
    }

    // Infinite loop
    while(1){    
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

                    w_Vx_m1_1 = write(Vx_m1, V_msg_p, 1); // writing the number 0 on the pipe (0= Vx--)
                    if(w_Vx_m1_1 < 0) { 
                        perror("error tring to write on the Vx pipe from cmd (Vx--)"); // checking errors
                        logger(log_pn_command, "e0010"); // write a error log message
                    }
                    else if(w_Vx_m1_1 > 0){
                        logger(log_pn_command, "0010"); // write a log message
                    }
                    sleep(1); // wait for one second
                }

                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                    refresh();

                    w_Vx_m1_2 = write(Vx_m1, (V_msg_p + 2), 1); // writing the number 2 on the pipe (2= Vx++)
                    if(w_Vx_m1_2 < 0) { 
                        perror("error tring to write on the Vx pipe from cmd (Vx++)"); // checking errors
                        logger(log_pn_command, "e0011"); // write a error log message
                    }
                    else if(w_Vx_m1_2 > 0) {
                        logger(log_pn_command, "0011"); // write a log message
                    }
                    sleep(1); // wait for one second
                }

                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                    refresh();

                    w_Vx_m1_3 = write(Vx_m1, (V_msg_p + 1), 1); // writing the number 1 on the pipe (1= Vx=0)
                    if(w_Vx_m1_3 < 0) { 
                        perror("error tring to write on the Vx pipe from cmd (Vx=0)"); // checking errors
                        logger(log_pn_command, "e0100"); // write a error log message                        
                    }
                    else if (w_Vx_m1_3 > 0){
                        logger(log_pn_command, "0100"); // write a log message
                    }
                    sleep(1); // wait for one second
                }

                // Vz-- button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                    refresh();

                    w_Vz_m2_1 = write(Vz_m2, V_msg_p, 1); // writing the number 0 on the pipe (0= Vz--)
                    if(w_Vz_m2_1 < 0) { 
                        perror("error tring to write on the Vz pipe from cmd (Vz--)"); // checking errors
                        logger(log_pn_command, "e0101"); // write a error log message
                    }
                    else if (w_Vz_m2_1 > 0){
                        logger(log_pn_command, "0101"); // write a log message
                    }
                    sleep(1); // wait for one second
                }

                // Vz++ button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                    refresh();

                    w_Vz_m2_2 = write(Vz_m2, (V_msg_p + 2), 1); // writing the number 2 on the pipe (2= Vz++)
                    if(w_Vz_m2_2 < 0) { 
                        perror("error tring to write on the Vz pipe from cmd (Vz++)"); // checking errors
                        logger(log_pn_command, "e0110"); // write a error log message
                    }
                    else if (w_Vz_m2_2 > 0) {
                        logger(log_pn_command, "0110"); // write a log message
                    }
                    sleep(1); // wait for one second
                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
                    refresh();

                    w_Vz_m2_3 = write(Vz_m2, (V_msg_p + 1), 1); // writing the number 1 on the pipe (1= Vz=0)
                    if(w_Vz_m2_3 < 0) { 
                        perror("error tring to write on the Vz pipe from cmd (Vz=0)"); // checking errors
                        logger(log_pn_command, "e0111"); // write a error log message
                    }
                    else if (w_Vz_m2_3 > 0){
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
