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
#include <signal.h>
#include <errno.h>

// declare some global variables:
int x_e_in; // declare the file descriptor of the pipe x
int z_e_in; // declare the file descriptor of the pipe z
int x_e_out; // declare the file descriptor of the pipe x_c (corrected)
int z_e_out; // declare the file descriptor of the pipe z_c (corrected)
const char *x = "./bin/named_pipes/x"; // initialize the pipe x pathname
const char *z = "./bin/named_pipes/z"; // initialize the pipe z pathname
const char *x_c = "./bin/named_pipes/x_c"; // initialize the pipe x_c pathname
const char *z_c = "./bin/named_pipes/z_c"; // initialize the pipe x_c pathname

void sig_handler (int signo) {
    if (signo == SIGTERM) {
        if (close(x_e_in) < 0) { // close the pipe x
            perror("error closing the pipe x from error proces"); // checking errors
        }
        if (close(z_e_in) < 0) { // close the pipe z
            perror("error closing the pipe z from error proces"); // checking errors
        }
        if (close(x_e_out) < 0) { // close the pipe x_c
            perror("error closing the pipe x_c from error proces"); // checking errors
        }
        if (close(z_e_out) < 0) { // close the pipe z_c
            perror("error closing the pipe z_c from error proces"); // checking errors
        }
        if (unlink(x) < 0) { // delete the file name from the system of the pipe x
            perror("error deleting the pipe x from error proces"); // checking errors
        }
        if (unlink(z) < 0) { // delete the file name from the system of the pipe z
            perror("error deleting the pipe z from error proces"); // checking errors
        }
        if (unlink(x_c) < 0) { // delete the file name from the system of the pipe x_c
            perror("error deleting the pipe x_c from error proces"); // checking errors
        }
        if (unlink(z_c) < 0) { // delete the file name from the system of the pipe z_c
            perror("error deleting the pipe z_c from error proces"); // checking errors
        }
        if (raise(SIGKILL) != 0) { // proces commit suicide
            perror("error suiciding the error proces"); // checking errors
        }
    }
}

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

int main(int argc, char const *argv[]) {
    float x_rcv[4]; // declare the x position receiving buffer
    float * x_rcv_p = &x_rcv[0]; // initialize the pointer to the x_rcv array
    float z_rcv[4]; // declare the z position receiving buffer
    float * z_rcv_p = &z_rcv[0]; // initialize the pointer to the z_rcv array

    float x_snd[4]; // declare the x corrected position sending buffer
    float * x_snd_p = &x_snd[0]; // initialize the pointer to the x_snd array
    float z_snd[4]; // declare the z corrected position sending buffer
    float * z_snd_p = &z_snd[0]; // initialize the pointer to the z_snd array

    fd_set rfds; // declare the select mode
    struct timeval tv; // declare the time interval of the select function
    int retval; // declare the returned valeu
    int nfds; // declaring number of fd

    float e; // declare the random number

    float x_e; // declare the internal computed and used x
    float x_e_i; // declare the internal x used for the evaluations
    float z_e; // declare the internal computed and used z
    float z_e_i; // declare the internal z used for the evaluations
    float x_prev = 0.000000; // declare the previous valeu received of x
    float x_e_prev = 0.000000; // declare the previous valeu setted of x
    float z_prev = 0.000000; // declare the previous valeu received of z
    float z_e_prev = 0.000000; // declare the previous valeu setted of z

    int sel_err; // declaring the returned valeu of the function select
    ssize_t r_x_e_in; // declaring the returned valeu of the read function on the pipe x
    ssize_t r_z_e_in; // declaring the returned valeu of the read function on the pipe z

    ssize_t w_x_e_out; // declaring the returned valeu of the write function on the pipe x_c
    ssize_t w_z_e_out; // declaring the returned valeu of the write function on the pipe z_c

    const char * log_pn_error = "./bin/log_files/error.txt"; // initialize the log file path name

    remove(log_pn_error); // remove the previous log file
    logger(log_pn_error, "log legend:  0001=opened the pipes  0010= x received and error computed  0011 = x exceed upper bound  0100= x exceed lower bound  0101= x sended to inspect  0110= z received and error computed  0111= z exceed upper bound  1000= z exceed the lower bound  1001= z sended to the inspect  1010= selet sucseed  1011= select timer elapsed before receiving data  1100= closure signal received.    the log number with an e in front means the relative operation failed");

    if (signal(SIGTERM, sig_handler) == SIG_ERR) { // check if there is any closure signal
        perror("error receiving the closure signal from the master in error"); // checking errors
        logger(log_pn_error, "e1100"); // write a error log message
    }

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

    // chose the right nfds based on the fd
    if (x_e_in > z_e_in) {
        nfds = x_e_in + 1;
    }
    else {
        nfds = z_e_in + 1;
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
        tv.tv_usec = 0; // set the time interval in microseconds

        sel_err = select(nfds, &rfds, NULL, NULL, &tv); // checking if there is any pipe avaiable for the reading
        if (sel_err < 0 && errno != EAGAIN) { 
            perror("error in the select of error proces"); // checking errors
            logger(log_pn_error, "e1010"); // write a error log message
        }
        else if (sel_err == 0){
            logger(log_pn_error, "1011"); // write a log message
        }
        else if (sel_err > 0){
            logger(log_pn_error, "1010"); // write a log message
            if (FD_ISSET(x_e_in, &rfds) > 0) { // the pipe x_e_in is ready for communicate
                r_x_e_in = read(x_e_in, x_rcv_p, 4); // reading the pipe
                if(r_x_e_in < 0) {
                    perror("error reading the pipe x from error proces"); // checking errors
                    logger(log_pn_error, "e0010"); // write a error log message
                }
                else if (r_x_e_in > 0) { // otherwise read the position x and add the correction of the error
                    // pick the received valeu
                    x_e_i = x_rcv[0];

                    if (x_e_i == x_prev) {
                        x_e = x_e_prev; // maintain the previous error
                    }
                    else {
                        // setting up the random variables
                        e = ((rand() % (100 + 1)) - 50) / 100.00; // compute the error
                        x_e = x_e_i + e;
                    }
                    x_prev = x_e_i; // setting the valeu of x for the next iteration
                    x_e_prev = x_e; 

                    logger(log_pn_error, "0010"); // write a log message

                    if (x_e > 40){
                        x_e = 40;
                        logger(log_pn_error, "0011"); // write a log message
                    }
                    else if (x_e < 0) {
                        x_e = 0;
                        logger(log_pn_error, "0100"); // write a log message
                    }

                    x_snd[0] = x_e;

                    w_x_e_out = write(x_e_out, x_snd_p, 4); // writing the corrected position on the pipe
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
                r_z_e_in = read(z_e_in, z_rcv_p, 4); // reading the pipe
                if(r_z_e_in <= 0) { 
                    perror("error reading the pipe z from error proces"); // checking errors
                    logger(log_pn_error, "e0010"); // write a error log message 
                }
                else if (r_z_e_in > 0 ) { // otherwise read the position z and add the correction of the error
                    // pick the received valeu
                    z_e_i = z_rcv[0];

                    if (z_e_i == z_prev) {
                        z_e = z_e_prev; // maintain the previous error
                    }
                    else {
                        // setting up the random variables
                        e = ((rand() % (100 + 1)) - 50) / 100.00; // compute the error
                        z_e = z_e_i + e;
                    }
                    z_prev = z_e_i; // setting the valeu of z for the next iteration
                    z_e_prev = z_e;

                    logger(log_pn_error, "0110"); // write a log message

                    if (z_e > 10) {
                        z_e = 10;
                        logger(log_pn_error, "0111"); // write a log message
                    }
                    else if (z_e < 0) {
                        z_e = 0;
                        logger(log_pn_error, "1000"); // write a log message
                    }

                    z_snd[0] = z_e;

                    w_z_e_out = write(z_e_out, z_snd_p, 4); // writing the corrected position on the pipe
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