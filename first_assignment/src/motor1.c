#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>

int Vx_m1; // inizialize the file descriptor of the pipe Vx
int x_m1; // inizialize the file descriptor of the pipe x

char *Vx = "/named_pipes/Vx";// initialize the pipe Vx pathname
char *x = "/named_pipes/x"; // initialize the pipe x pathname
int Vx_rcv[1]; // initialize the buffer where i will store the received variable from the pipe Vx
int x_snd[1]; // initialize the buffer where i will send the position x

int x_i = 0; // initialize position along x axis
int Vx_i = 0; // initialize the velocity along x
int T = 10; // initialize the time period of the speed

int main(int argc, char const *argv[]) {

    // open the pipes:
    Vx_m1 = open(Vx, O_RDONLY); // open the pipe Vx to read on it
    if(Vx_m1 < 0){
        perror("error opening the pipe Vx from m1"); // checking errors
    }

    x_m1 = open(x, O_WRONLY); // open the pipe x to write on it
    if(x_m1 < 0){
        perror("error while opening the pipe x from m1"); // checking errors
    }

    while(1){
        if(read(Vx_m1, Vx_rcv, sizeof(Vx_rcv)) < 0) {
            perror("error reading the pipe Vx from m1"); // checking errors
        }
        else if (read(Vx_m1, Vx_rcv, sizeof(Vx_rcv)) == 0) { // no message received
            x_i = x_i + (Vx_i * T);
            sleep(T);
        }
        else if (read(Vx_m1, Vx_rcv, sizeof(Vx_rcv)) > 0 && x_i >= 0 && x_i <= 100) { 
            if (Vx_rcv[0] == 0) { // decrease the velocity
                Vx_i = Vx_i - 4;
                x_i = x_i + (Vx_i * T);
                sleep(T);
            }

            else if (Vx_rcv[0] == 1) { // set velocity equal to 0
                Vx_i = 0;
                sleep(T);
            }

            else if (Vx_rcv[0] == 2) { // increase the velocity
                Vx_i = Vx_i + 4;
                x_i = x_i + (Vx_i * T);
                sleep(T);
            }
        }
        else if (x_i > 100) { // reached upper bound stop at that position
            x_i = 100;
        }

        else if (x_i < 0) { // reached lower bound stop at that position
            x_i = 0;
        }
        

        x_snd[0] = x_i; // putting the position x_i in the buffer to send it

        if(write(x_m1, x_snd, sizeof(x_snd)) != 1) { // writing the position on the pipe
            perror("error tring to write on the x pipe from m1"); // checking errors
        }
    }
}