#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <stdio.h>

int x_e; // declare the file descriptor of the pipe x
int z_e; // declare the file descriptor of the pipe z
char *x = "/named_pipes/x"; // initialize the pipe x pathname
char *z = "/named_pipes/z"; // initialize the pipe x pathname

fd_set rfds; // declare the select mode
struct timeval tv; // declare the time interval of the select function
int retval; // declare the returned valeu, which will be the fd on which we will read
int nfds = 1; // initialize number of fd starting from 0
int fd; // declare the counter for FD_ISSET

int e; // declare the random number
int rnum_u = 0; // initialize the upper bound for the random number
int rnum_l = 10; // initialize the lower bound for the random number

int main(int argc, char const *argv[]) {
    // open the pipes:
    x_e = open(x, O_RDONLY); // open the pipe x to read on it
    if(x_e < 0){
        perror("error opening the pipe x from error"); // checking errors
    }

    z_e = open(z, O_RDONLY); // open the pipe z to read on it
    if(z_e < 0){
        perror("error opening the pipe z from error"); // checking errors
    }
    while(1){
        // setting up the select
        FD_ZERO(&rfds); // clear all the fd in the set
        FD_SET(x_e, &rfds); // put the fd of x_e in the set
        FD_SET(z_e, &rfds); // put the fd of z_e in the set
        tv.tv_sec = 5; // set the time interval in seconds
        tv.tv_usec = 0; // set the time interval in microseconds
        if( select((nfds+1), &rfds, NULL, NULL, &tv) < 0) { // checking errors
            perror("error in the select of error proces");
        };
        for (fd = 0, fd <= nfds, fd++) {
            if (FD_ISSET(fd, &rfds)) { // checking which pipe is ready for communications
                if (fd == x_e) { // the pipe x_e is ready for communicate 
                    e = (rand() % (upper - lower + 1)) + lower;
                }
                else if (fd == z_e) { // the pipe z_e is ready for communicate
                    isisis
                }
                else { // the timer elapsed without receiving new data
                    perror("no data received in error proces");
                }
            }
        }
    }



    /*
    z_e = open(z, O_WRONLY); // open the pipe z to write on it
    if(z_m2 < 0){
        perror("error while opening the pipe z from m2"); // checking errors
    }
    */
}