#include "./../include/inspection_utilities.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int logger(char * log_pathname, char log_msg[]) {
  int log_fd; // declare the log file descriptor
  char log_msg_arr[strlen(log_msg)+11]; // declare the message string
  double c = (double) (clock() / CLOCKS_PER_SEC); // evaluate the time from the program launch
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

int main(int argc, char const *argv[]){
    int x_ins_in; // declare the file descriptor of the pipe x_c
    int z_ins_in; // declare the file descriptor of the pipe z_c
    int s_ins_out; // declare the file descriptor of the pipe s 
    char *x_c = "./bin/named_pipes/x_c"; // initialize the pipe x_c pathname
    char *z_c = "./bin/named_pipes/z_c"; // initialize the pipe z_c pathname
    char *s = "./bin/named_pipes/s"; // initialize the the pipe s pathname

    float x_rcv[4]; // declare the x position receiving buffer
    float * x_rcv_p = &x_rcv[0]; // initialize the pointer to the x_rcv array
    float z_rcv[4]; // declare the z position receiving buffer
    float * z_rcv_p = &z_rcv[0]; // initialize the pointer to the z_rcv array
    int s_snd[1]; // declare the s signal sending buffer
    int * s_snd_p = &s_snd[0]; // initialize the pointer to the s_snd array

    fd_set rfds; // declare the select mode
    struct timeval tv; // declare the time interval of the select function
    int retval; // declare the returned valeu
    int nfds; // declaring the number of fd
    int fd; // declare the counter for FD_ISSET

    float x, z; // End-effector coordinates

    int s_ins; // declaring the returned valeu of the function select
    int r_x_ins_in; // declaring the returned valeu of the read function on the pipe x_c
    int r_z_ins_in; // declaring the returned valeu of the read function on the pipe z_c
    int w_s_ins_out_1; // declaring the returned valeu of the write function on the pipe s, for the stop signal
    int w_s_ins_out_2; // declaring the returned valeu of the write function on the pipe s, for the reset signal

    char * log_pn_inspect = "./bin/log_files/inspect.txt"; // initialize the log file path name
    remove(log_pn_inspect); // remove the previous log file
    logger(log_pn_inspect, "log legend:  0001=opened the pipes  0010= x received  0011= z received  0100= stop signal sended  0101= reset signal sended  1010= timer elapsed without new messages.    the log number with an e in front means the relative operation failed");

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize User Interface 
    init_console_ui();

    // open the pipes:
    // input pipes:
    x_ins_in = open(x_c, O_RDONLY | O_NONBLOCK); // open the pipe x_c to read on it
    if(x_ins_in < 0){
        perror("error opening the pipe x_c from inspection"); // checking errors
    }

    z_ins_in = open(z_c, O_RDONLY | O_NONBLOCK); // open the pipe z_c to read on it
    if(z_ins_in < 0){
        perror("error opening the pipe z_c from inspection"); // checking errors
    }

    // chose the right nfds based on the fd
    if (x_ins_in > z_ins_in) {
        nfds = x_ins_in + 1;
    }
    else {
        nfds = z_ins_in + 1;
    }

    // output pipes:
    s_ins_out = open(s, O_WRONLY); // open the pipe s to write on it
    if(s_ins_out < 0){
        perror("error opening the pipe s from inspection"); // checking errors
    }

    if (x_ins_in > 0 && z_ins_in > 0 && s_ins_out > 0) {
        logger(log_pn_inspect, "0001"); // write a log message
    }
    else {
        logger(log_pn_inspect, "e0001"); // write a error log message
    }

    // Infinite loop
    while(1){
        // clear the array where i will store the data
        memset(x_rcv_p, 0, sizeof(x_rcv)); // clear the receiving messages array
        memset(z_rcv_p, 0, sizeof(z_rcv)); // clear the receiving messages array
        
        // setting up the select
        FD_ZERO(&rfds); // clear all the fd in the set
        FD_SET(x_ins_in, &rfds); // put the fd of x_ins_in in the set
        FD_SET(z_ins_in, &rfds); // put the fd of z_ins_in in the set
        tv.tv_sec = 0; // set the time interval in seconds
        tv.tv_usec = 50; // set the time interval in microseconds

        s_ins = select((nfds+1), &rfds, NULL, NULL, &tv);
        if (s_ins < 0) { 
            perror("error in the select of inspect"); // checking errors
            logger(log_pn_inspect, "e0010"); // write a error log message
        }
        else if (s_ins == 0){
            //perror("timer elapsed in inspect select without data");
            logger(log_pn_inspect, "1010"); // write a error log message
        }
        else if (s_ins > 0) {
            if (FD_ISSET(x_ins_in, &rfds) > 0) { // the pipe x_ins_in is ready for communicate
                r_x_ins_in = read(x_ins_in, x_rcv_p, 4); // reading the pipe
                if(r_x_ins_in <= 0) { 
                    perror("error reading the pipe x_c from inspect"); // checking errors
                    logger(log_pn_inspect, "e0010"); // write a error log message
                }
                else if (r_x_ins_in > 0) { // otherwise read the position x
                    x = x_rcv[0];

                    logger(log_pn_inspect, "0010"); // write a log message
                }
            }
            
            else if (FD_ISSET(z_ins_in, &rfds) > 0) { // the pipe z_ins_in is ready for communicate
                r_z_ins_in = read(z_ins_in, z_rcv_p, 4); // reading the pipe
                if(r_z_ins_in <= 0) {
                    perror("error reading the pipe z_c from inspect"); // checking errors
                    logger(log_pn_inspect, "e0011"); // write a error log message 
                }
                else if (r_z_ins_in > 0) { // otherwise read the position z
                    z = z_rcv[0];

                    logger(log_pn_inspect, "0011"); // write a log message
                }
            }

        }

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

                // STOP button pressed
                if(check_button_pressed(stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "STP button pressed");
                    refresh();
                    s_snd[0] = 0; // setting the signal id to send
                    w_s_ins_out_1 = write(s_ins_out, s_snd_p, 1); // writing the signal id on the pipe
                    if(w_s_ins_out_1 <= 0) { 
                        perror("error tring to write on the s pipe from inspect"); // checking errors
                        logger(log_pn_inspect, "e0100"); // write a error log message
                    }
                    else if (w_s_ins_out_1 > 0){
                        logger(log_pn_inspect, "0100"); // write a log message
                    }

                    sleep(1);
                }

                // RESET button pressed
                else if(check_button_pressed(rst_button, &event)) {
                    mvprintw(LINES - 1, 1, "RST button pressed");
                    refresh();
                    s_snd[0] = 1; // setting the signal id to send
                    w_s_ins_out_2 = write(s_ins_out, s_snd_p, sizeof(s_snd)); // writing the signal id on the pipe
                    if(w_s_ins_out_2 <= 0) { 
                        perror("error tring to write on the s pipe from inspect"); // checking errors
                        logger(log_pn_inspect, "e0101"); // write a error log message
                    }
                    else if (w_s_ins_out_2 > 0) {
                        logger(log_pn_inspect, "0101"); // write a log message
                    }

                    sleep(1);
                }
            }
        }
    
        // Update UI
        update_console_ui(&x, &z);
	}

    // Terminate
    endwin();
    return 0;
}
