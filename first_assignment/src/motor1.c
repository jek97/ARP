#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

int Vx_m1; // inizialize the file descriptor of the pipe Vx
int x_m1; // inizialize the file descriptor of the pipe x

char *Vx = "/named_pipes/Vx";// initialize the pipe Vx pathname
char *x = "/named_pipes/x"; // initialize the pipe x pathname
int Vx_rcv[1]; // initialize the buffer where i will store the received variable from the pipe Vx
int x_snd[1]; // initialize the buffer where i will send the position x

int x_i = 0; // initialize position along x axis
int Vx_i = 0; // initialize the velocity along x
int T = 10; // initialize the time period of the speed

void sig_handler (int signo) {
    if (signo == SIGUSR1) { // stop signal received
        Vx_i = 0; // set the velocity to 0
    }
    else if (signo == SIGUSR2) { //reset signal received
        Vx_i = 0; // set the velocity to 0
        x_i = -0.1; // set the positionx to 0, , thanks to the error proces also
    }
}

void logger(char * log_pathname, char log_msg[]) {
  double c = (double) (clock() / CLOCKS_PER_SEC);
  char log_msg_arr[sizeof(&log_msg)+11];
  if ((sprintf(log_msg_arr, " %s,%.2E;", log_msg, c)) < 0){
    perror("error in logger sprintf");
  }
  int log_fd; // declare the log file descriptor
  if ((log_fd = open(log_pathname, O_WRONLY | O_CREAT | O_APPEND, 00700)) < 0){ // open the log file to write on it
    perror("error opening the log file"); // checking errors
  }
  if(write(log_fd, log_msg_arr, sizeof(log_msg_arr)) != sizeof(log_msg_arr)) { // writing the log message on the log file
      perror("error tring to write the log message in the log file"); // checking errors
  }
}

int main(int argc, char const *argv[]) {
    //log legend: /n 0001=opened the pipes /n 0010= no message received /n 0011 = decrease velocity /n 0100= velocity=0 /n 0101= increase velocity /n 0110= reached upper bound /n 0111= reached lower bound /n 1000= writed the position on the pipe

    // condition for the signal:
    if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
        perror("error receiving the signal from command_console");
    }
    if (signal(SIGUSR2, sig_handler) == SIG_ERR) {
        perror("error receiving the signal from command_console");
    }

    // open the pipes:
    Vx_m1 = open(Vx, O_RDONLY); // open the pipe Vx to read on it
    if(Vx_m1 < 0){
        perror("error opening the pipe Vx from m1"); // checking errors
    }

    x_m1 = open(x, O_WRONLY); // open the pipe x to write on it
    if(x_m1 < 0){
        perror("error while opening the pipe x from m1"); // checking errors
    }

    logger("./log_files/motor1.txt", "0001"); // write a log message

    while(1){
        if(read(Vx_m1, Vx_rcv, sizeof(Vx_rcv)) < 0) {
            perror("error reading the pipe Vx from m1"); // checking errors
        }
        else if (read(Vx_m1, Vx_rcv, sizeof(Vx_rcv)) == 0) { // no message received
            x_i = x_i + (Vx_i * T);
            logger("./log_files/command.txt", "0010"); // write a log message
        }
        else if (read(Vx_m1, Vx_rcv, sizeof(Vx_rcv)) > 0 && x_i >= 0 && x_i <= 100) { 
            if (Vx_rcv[0] == 0) { // decrease the velocity
                Vx_i = Vx_i - 4;
                x_i = x_i + (Vx_i * T);
                logger("./log_files/command.txt", "0011"); // write a log message
            }

            else if (Vx_rcv[0] == 1) { // set velocity equal to 0
                Vx_i = 0;
                logger("./log_files/command.txt", "0100"); // write a log message
            }

            else if (Vx_rcv[0] == 2) { // increase the velocity
                Vx_i = Vx_i + 4;
                x_i = x_i + (Vx_i * T);
                logger("./log_files/command.txt", "0101"); // write a log message
            }
        }
        else if (x_i > 100) { // reached upper bound stop at that position
            x_i = 100;
            logger("./log_files/command.txt", "0110"); // write a log message
        }

        else if (x_i < 0) { // reached lower bound stop at that position
            x_i = 0;
            logger("./log_files/command.txt", "0111"); // write a log message
        }
        sleep(T);

        x_snd[0] = x_i; // putting the position x_i in the buffer to send it

        if(write(x_m1, x_snd, sizeof(x_snd)) != 1) { // writing the position on the pipe
            perror("error tring to write on the x pipe from m1"); // checking errors
        }
        logger("./log_files/command.txt", "1000"); // write a log message
    }
}