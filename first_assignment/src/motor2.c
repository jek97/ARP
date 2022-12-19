#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>

int z_i = 0; // initialize position along z axis
int Vz_i = 0; // initialize the velocity along z

void sig_handler (int signo) {
    if (signo == SIGUSR1) { // stop signal received
        Vz_i = 0; // set the velocity to 0
       
    }
    else if (signo == SIGUSR2) { //reset signal received
        Vz_i = 0; // set the velocity to 0
        z_i = -0.1; // set the position z to 0, thanks to the error proces also
        
    }
}

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

int main(int argc, char const *argv[]) {

    int Vz_m2; // inizialize the file descriptor of the pipe Vz
    int z_m2; // inizialize the file descriptor of the pipe z

    char *Vz = "./bin/named_pipes/Vz";// initialize the pipe Vz pathname
    char *z = "./bin/named_pipes/z"; // initialize the pipe z pathname
    int Vz_rcv[1]; // initialize the buffer where i will store the received variable from the pipe Vz
    int z_snd[1]; // initialize the buffer where i will send the position z
    int T = 1; // initialize the time period of the speed

    char * log_pn_motor2 = "./bin/log_files/motor2.txt"; // initialize the log file path name
    remove(log_pn_motor2);
    logger(log_pn_motor2, "log legend:  0001=opened the pipes  0010= no message received  0011 = decrease velocity  0100= velocity=0  0101= increase velocity  0110= reached upper bound  0111= reached lower bound  1000= writed the position on the pipe  1001=stop signal received  1010= reset signal received.    the log number with an e in front means the relative operation failed");

    // condition for the signal:
    if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
        perror("error receiving the signal from command_console");
        logger(log_pn_motor2, "e1001"); // write a error log message
    }

    if (signal(SIGUSR2, sig_handler) == SIG_ERR) {
        perror("error receiving the signal from command_console");
        logger(log_pn_motor2, "e1010"); // write a error log message
    }
    
    // open the pipes:
    Vz_m2 = open(Vz, O_RDONLY | O_NONBLOCK); // open the pipe Vz to read on it
    if(Vz_m2 < 0){
        perror("error opening the pipe Vz from m2"); // checking errors
    }

    z_m2 = open(z, O_WRONLY); // open the pipe z to write on it
    if(z_m2 < 0){
        perror("error while opening the pipe z from m2"); // checking errors
    }

    if (Vz_m2 > 0 && z_m2 > 0) {
        logger(log_pn_motor2, "0001"); // write a log message
    }
    else {
        logger(log_pn_motor2, "e0001"); // write a errorlog message
    }
    while(1){

        // read the pipe and compute the position along x
        if(read(Vz_m2, Vz_rcv, sizeof(Vz_rcv)) < 0) {
            perror("error reading the pipe Vz from m2"); // checking errors
            logger(log_pn_motor2, "e0010"); // write a error log message
            
        }
        else if (read(Vz_m2, Vz_rcv, sizeof(Vz_rcv)) == 0) { // no message received
            z_i = z_i + (Vz_i * T);
            logger(log_pn_motor2, "0010"); // write a log message
        }
        else { 
            if (Vz_rcv[0] == 0) { // decrease the velocity
                Vz_i = Vz_i - 4;
                z_i = z_i + (Vz_i * T);
                logger(log_pn_motor2, "0011"); // write a log message
            }

            else if (Vz_rcv[0] == 1) { // set velocity equal to 0
                Vz_i = 0;
                logger(log_pn_motor2, "0100"); // write a log message
            }

            else if (Vz_rcv[0] == 2) { // increase the velocity
                Vz_i = Vz_i + 4;
                z_i = z_i + (Vz_i * T);
                logger(log_pn_motor2, "0101"); // write a log message
            }
        }
        
        // constrol if z reached the upper bound:
        if (z_i > 100) { // reached upper bound stop at that position
            z_i = 100;
            Vz_i = 0;
            logger(log_pn_motor2, "0110"); // write a log message
        }

        else if (z_i < 0) { // reached lower bound stop at that position
            z_i = 0;
            Vz_i = 0;
            logger(log_pn_motor2, "0111"); // write a log message
        }
        // wait to simulate the speed:
        sleep(T);
        // write the position in the bufer and then on the pipe:
        z_snd[0] = z_i; // putting the position z_i in the buffer to send it

        if(write(z_m2, z_snd, sizeof(z_snd)) != sizeof(z_snd)) { // writing the position on the pipe
            perror("error tring to write on the z pipe from m2"); // checking errors
            logger(log_pn_motor2, "e1000"); // write a error log message
        }
        else {
            logger(log_pn_motor2, "1000"); // write a log message
        }
    }
}