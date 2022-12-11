#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int x_e_in; // declare the file descriptor of the pipe x
int z_e_in; // declare the file descriptor of the pipe z
char *x = "/named_pipes/x"; // initialize the pipe x pathname
char *z = "/named_pipes/z"; // initialize the pipe z pathname

int x_e_out; // declare the file descriptor of the pipe x_c (corrected)
int z_e_out; // declare the file descriptor of the pipe z_c (corrected)
char *x_c = "/named_pipes/x_c"; // initialize the pipe x_c pathname
char *z_c = "/named_pipes/z_c"; // initialize the pipe x_c pathname

int x_rcv[1]; // declare the x position receiving buffer
int z_rcv[1]; // declare the z position receiving buffer

int x_snd[1]; // declare the x corrected position sending buffer
int z_snd[1]; // declare the z corrected position sending buffer

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


int main(int argc, char const *argv[]) {
    // open the pipes:
    // input pipes
    x_e_in = open(x, O_RDONLY); // open the pipe x to read on it
    if(x_e_in < 0){
        perror("error opening the pipe x from error"); // checking errors
    }

    z_e_in = open(z, O_RDONLY); // open the pipe z to read on it
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


    // computing
    while(1){
        // setting up the select
        FD_ZERO(&rfds); // clear all the fd in the set
        FD_SET(x_e_in, &rfds); // put the fd of x_e_in in the set
        FD_SET(z_e_in, &rfds); // put the fd of z_e_in in the set
        tv.tv_sec = 5; // set the time interval in seconds
        tv.tv_usec = 0; // set the time interval in microseconds

        // setting up the random variables
        rnum_u_x = (int)(x_rcv[0] * 2); // seting the upper bound for the x error
        rnum_u_z = (int)(z_rcv[0] * 2); // setting the upper bound for the z error

        if (select((nfds+1), &rfds, NULL, NULL, &tv) < 0) { // checking errors
            perror("error in the select of error proces");
        }
        else if (select((nfds+1), &rfds, NULL, NULL, &tv) == 0){
            perror("timer elapsed in error proces input select without data");
        }
        else{
            for (fd = 0; fd <= nfds; fd++) {
                if (FD_ISSET(fd, &rfds)) { // checking if there is a pipe readyfor communications
                    if (fd == x_e_in) { // the pipe x_e_in is ready for communicate
                        if(read(x_e_in, x_rcv, sizeof(x_rcv)) < 0) { // checking errors
                            perror("error reading the pipe x from error proces"); 
                        }
                        else if (read(x_e_in, x_rcv, sizeof(x_rcv)) > 0) { // otherwise read the position x and add the correction of the error
                            e = ((rand() % (rnum_u_x + 1))-(rnum_u_x/2)) / 10;
                            x_e = x_rcv[0] + e;
                            if (x_e > 100){
                                x_e = 100;
                            }
                            else if (x_e < 0) {
                                x_e = 0;
                            }
                            x_snd[0] = x_e;
                            if(write(x_e_out, x_snd, sizeof(x_snd)) != 1) { // writing the corrected position on the pipe
                                perror("error tring to write on the x_c pipe from error proces"); // checking errors
                            }
                        }
                    }
                    if (fd == z_e_in) { // the pipe z_e_in is ready for communicate
                        if(read(z_e_in, z_rcv, sizeof(z_rcv)) < 0) { // checking errors
                            perror("error reading the pipe z from error proces"); 
                        }
                        else if (read(z_e_in, z_rcv, sizeof(z_rcv)) > 0) { // otherwise read the position z and add the correction of the error
                            e = ((rand() % (rnum_u_z + 1))-(rnum_u_z/2)) / 10;
                            z_e = z_rcv[0] + e;
                            if (z_e >100) {
                                z_e = 100;
                            }
                            else if (z_e < 0) {
                                z_e = 0;
                            }
                            z_snd[0] = z_e;
                            if(write(z_e_out, z_snd, sizeof(z_snd)) != 1) { // writing the corrected position on the pipe
                                perror("error tring to write on the z_c pipe from error proces"); // checking errors
                            }
                        }
                    }
                }
            }
        }
        
    }
}