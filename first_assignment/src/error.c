#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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

int main(int argc, char const *argv[]) {

    int x_e_in; // declare the file descriptor of the pipe x
    int z_e_in; // declare the file descriptor of the pipe z
    char *x = "./bin/named_pipes/x"; // initialize the pipe x pathname
    char *z = "./bin/named_pipes/z"; // initialize the pipe z pathname

    int x_e_out; // declare the file descriptor of the pipe x_c (corrected)
    int z_e_out; // declare the file descriptor of the pipe z_c (corrected)
    char *x_c = "./bin/named_pipes/x_c"; // initialize the pipe x_c pathname
    char *z_c = "./bin/named_pipes/z_c"; // initialize the pipe x_c pathname

    int x_rcv[1]; // declare the x position receiving buffer
    int * x_rcv_p = &x_rcv[0]; // initialize the pointer to the x_rcv array
    int z_rcv[1]; // declare the z position receiving buffer
    int * z_rcv_p = &z_rcv[0]; // initialize the pointer to the z_rcv array

    int x_snd[1]; // declare the x corrected position sending buffer
    int * x_snd_p = &x_snd[0]; // initialize the pointer to the x_snd array
    int z_snd[1]; // declare the z corrected position sending buffer
    int * z_snd_p = &z_snd[0]; // initialize the pointer to the z_snd array

    fd_set rfds; // declare the select mode
    struct timeval tv; // declare the time interval of the select function
    int retval; // declare the returned valeu
    int nfds = 1; // initialize number of fd starting from 0
    int fd; // declare the counter for FD_ISSET

    float e; // declare the random number
    int rnum_u_x; // declare the upper bound for the random number of x
    int rnum_u_z; // declare the upper bound for the random number of z

    float x_e; // declare the internal computed and used x
    float z_e; // declare the internal computed and used z

    int sel_err; // declaring the returned valeu of the function select
    int r_x_e_in; // declaring the returned valeu of the read function on the pipe x
    int r_z_e_in; // declaring the returned valeu of the read function on the pipe z

    int w_x_e_out; // declaring the returned valeu of the write function on the pipe x_c
    int w_z_e_out; // declaring the returned valeu of the write function on the pipe z_c

    char * log_pn_error = "./bin/log_files/error.txt"; // initialize the log file path name
    remove(log_pn_error); // remove the previous log file
    logger(log_pn_error, "log legend:  0001=opened the pipes  0010= x received and error computed  0011 = x exceed upper bound  0100= x exceed lower bound  0101= x sended to inspect  0110= z received and error computed  0111= z exceed upper bound  1000= z exceed the lower bound  1001= z sended to the inspect  1010= selet sucseed  1011= select timer elapsed before receiving data.    the log number with an e in front means the relative operation failed");

    // open the pipes:
    // input pipes
    x_e_in = open(x, O_RDONLY | O_NONBLOCK); // open the pipe x to read on it
    if(x_e_in < 0){
        perror("error opening the pipe x from error"); // checking errors
    }

    z_e_in = open(z, O_RDONLY | O_NONBLOCK); // open the pipe z to read on it
    if(z_e_in < 0){
        perror("error opening the pipe z from error"); // checking errors
    }

    // output pipes
    x_e_out = open(x_c, O_WRONLY); // open the pipe x_c to write on it
    if(x_e_out < 0){
        perror("error opening the pipe x_c from error"); // checking errors
    }

    z_e_out = open(z_c, O_WRONLY); // open the pipe z_c to read on it
    if(z_e_out < 0){
        perror("error opening the pipe z_c from error"); // checking errors
    }

    if (x_e_in > 0 && z_e_in > 0 && x_e_out > 0 && z_e_out > 0) {
        logger(log_pn_error, "0001"); // write a log message
    }
    else {
        logger(log_pn_error, "e0001"); // write a error log message
    }

    while(1){
        // setting up the select
        FD_ZERO(&rfds); // clear all the fd in the set
        FD_SET(x_e_in, &rfds); // put the fd of x_e_in in the set
        FD_SET(z_e_in, &rfds); // put the fd of z_e_in in the set
        tv.tv_sec = 1; // set the time interval in seconds
        tv.tv_usec = 50; // set the time interval in microseconds

        // setting up the random variables
        rnum_u_x = (int)(x_rcv[0] * 2); // seting the upper bound for the x error
        rnum_u_z = (int)(z_rcv[0] * 2); // setting the upper bound for the z error

        sel_err = select((nfds+1), &rfds, NULL, NULL, &tv); // checking if there is any pipe avaiable for the reading
        if (sel_err < 0) { 
            perror("error in the select of error proces"); // checking errors
            logger(log_pn_error, "e1010"); // write a error log message
        }
        else if (sel_err == 0){
            perror("timer elapsed in error proces input select without data"); // checking errors
            logger(log_pn_error, "1011"); // write a log message
        }
        else if (sel_err > 0){
            logger(log_pn_error, "1010"); // write a log message
            if (FD_ISSET(x_e_in, &rfds) > 0) { // the pipe x_e_in is ready for communicate
                r_x_e_in = read(x_e_in, x_rcv_p, 1); // reading the pipe
                if(r_x_e_in < 0) { 
                    perror("error reading the pipe x from error proces"); // checking errors
                    logger(log_pn_error, "e0010"); // write a error log message
                }
                else if (r_x_e_in > 0) { // otherwise read the position x and add the correction of the error
                    e = ((rand() % (rnum_u_x + 1))-(rnum_u_x/2)) / 10;
                    x_e = x_rcv[0] + e;

                    logger(log_pn_error, "0010"); // write a log message

                    if (x_e > 100){
                        x_e = 100;
                        logger(log_pn_error, "0011"); // write a log message
                    }
                    else if (x_e < 0) {
                        x_e = 0;
                        logger(log_pn_error, "0100"); // write a log message
                    }

                    x_snd[0] = x_e;
                    w_x_e_out = write(x_e_out, x_snd_p, 1); // writing the corrected position on the pipe
                    if(w_x_e_out <= 0) { 
                        perror("error tring to write on the x_c pipe from error proces"); // checking errors
                        logger(log_pn_error, "e0101"); // write a error log message
                    }
                    else if (w_x_e_out > 0) {
                        logger(log_pn_error, "0101"); // write a log message
                    }
                }
                memset(x_rcv_p, 0, sizeof(x_rcv)); // clear the receiving messages array
            }

            else if (FD_ISSET(z_e_in, &rfds) > 0) { // the pipe z_e_in is ready for communicate
                r_z_e_in = read(z_e_in, z_rcv_p, sizeof(z_rcv)); // reading the pipe
                if(r_z_e_in <= 0) { 
                    perror("error reading the pipe z from error proces"); // checking errors
                    logger(log_pn_error, "e0010"); // write a error log message 
                }
                else if (r_z_e_in > 0) { // otherwise read the position z and add the correction of the error
                    e = ((rand() % (rnum_u_z + 1))-(rnum_u_z/2)) / 10;
                    z_e = z_rcv[0] + e;

                    logger(log_pn_error, "0110"); // write a log message

                    if (z_e >100) {
                        z_e = 100;
                        logger(log_pn_error, "0111"); // write a log message
                    }
                    else if (z_e < 0) {
                        z_e = 0;
                        logger(log_pn_error, "1000"); // write a log message
                    }
                    z_snd[0] = z_e;
                    w_z_e_out = write(z_e_out, z_snd_p, 1); // writing the corrected position on the pipe
                    if(w_z_e_out <= 0) { 
                        perror("error tring to write on the z_c pipe from error proces"); // checking errors
                        logger(log_pn_error, "e1001"); // write a error log message
                    }
                    else if (w_z_e_out > 0){
                        logger(log_pn_error, "1001"); // write a log message
                    }
                }
                memset(z_rcv_p, 0, sizeof(z_rcv)); // clear the receiving messages array
            }
        }
    }
}