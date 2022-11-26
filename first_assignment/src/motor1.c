#include <unistd.h>

int Vx_m1; // inizialize the file descriptor of the pipe Vx
int x_m1; // inizialize the file descriptor of the pipe x
int Vx; // initialize the velocity along x
int Vx_rcv[1]; // initialize the buffer where i will store the received variable from the pipe Vx
int x = 0; // initialize position along x axis
int T = 10;

int main(int argc, char const *argv[]) {

    // open the pipes:
    Vx_m1 = open(Vx, O_RDONLY); // open the pipe Vx to read on it
    if(Vx_m1 < 0){
        perror('error while opening the pipe Vx from m1'); // checking errors
    }

    x_m1 = open(x, O_WRONLY); // open the pipe x to write on it
    if(x_m1 < 0){
        perror('error while opening the pipe x from m1'); // checking errors
    }

    while(TRUE){
        if(read(Vx_m1, Vx_rcv, sizeof(Vx_rcv)) < 0) {
            perror('error reading the pipe Vx from m1');
        }
        else if (read(Vx_m1, Vx_rcv, sizeof(Vx_rcv)) == 0) {
            perror('no proces has opened the Vx pipe for writing');
        }
        else if (x > 0 && x < 100){
            if (Vx_rcv[0] == 0) {
                Vx = Vx - 3;
                x = x + (Vx * T);
                sleep(T);
                if(write(x_m1, x, 1) != 1) { // writing the position on the pipe
                        perror('error tring to write on the x pipe from m1 (Vx--)'); // checking errors
                    }
            }
            if (Vx_rcv[0] == 1) {
                Vx = 0;
                sleep(T);
                if(write(x_m1, x, 1) != 1) { // writing the position on the pipe
                        perror('error tring to write on the x pipe from m1 (Vx=0)'); // checking errors
                    }
            }
            if (Vx_rcv[0] == 2) {
                Vx = Vx + 3;
                x = x + (Vx * T);
                sleep(T);
                if(write(x_m1, x, 1) != 1) { // writing the position on the pipe
                        perror('error tring to write on the x pipe from m1 (Vx++)'); // checking errors
                    }
            }
        }
        else if (x > 100) {
            x = 100;
            if(write(x_m1, x, 1) != 1) { // writing the position on the pipe
                perror('error tring to write on the x pipe from m1 (x=100)'); // checking errors
            }
        }
        else if (x < 0) {
            x = 0;
            if(write(x_m1, x, 1) != 1) { // writing the position on the pipe
                perror('error tring to write on the x pipe from m1 (x=0)'); // checking errors
            }
        }
    
    }
}