#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>

// declaring some global variables:
float z_i = 0; // initialize position along z axis
float Vz_i = 0; // initialize the velocity along z
int Vz_m2; // inizialize the file descriptor of the pipe Vz
int z_m2; // inizialize the file descriptor of the pipe z
const char *Vz = "./bin/named_pipes/Vz";// initialize the pipe Vz pathname
const char *z = "./bin/named_pipes/z"; // initialize the pipe z pathname

void sig_handler (int signo) {
    if (signo == SIGUSR1) { // stop signal received
        Vz_i = 0; // set the velocity to 0
        sleep(1);
    }
    else if (signo == SIGUSR2) { //reset signal received
        Vz_i = 0; // set the velocity to 0
        z_i = -0.1; // set the position z to 0, thanks to the error proces also
        sleep(1);
    }
    else if (signo == SIGTERM) {
        if (close(Vz_m2) < 0) { // close the pipe Vz
            perror("error closing the pipe Vz from motor2"); // checking errors
        }
        if (close(z_m2) < 0) { // close the pipe z
            perror("error closing the pipe z from motor2"); // checking errors
        }
        if (unlink(Vz) < 0) { // delete the file name from the system of the pipe Vz
            perror("error deleting the pipe Vz from motor2"); // checking errors
        }
        if (unlink(z) < 0) { // delete the file name from the system of the pipe z
            perror("error deleting the pipe z from motor2"); // checking errors
        }
        if (raise(SIGKILL) != 0) { // proces commit suicide
            perror("error suiciding the motor2"); // checking errors
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
    int Vz_rcv[1]; // initialize the buffer where i will store the received variable from the pipe Vz
    int * Vz_rcv_p = &Vz_rcv[0]; // initialize the pointer to the Vz_rcv array
    float z_snd[4]; // initialize the buffer where i will send the position z
    float * z_snd_p = &z_snd[0]; // initialize the pointer to the z_snd array
    int T = 1; // initialize the time period of the speed

    ssize_t r_Vz_m2; // declaring the returned valeu of the read function on the pipe Vz
    ssize_t w_z_m2; // declaring the returned valeu of the write function on the pipe z

    const char * log_pn_motor2 = "./bin/log_files/motor2.txt"; // initialize the log file path name
    remove(log_pn_motor2); // remove the previous log file
    logger(log_pn_motor2, "log legend:  0001=opened the pipes  0010= no message received  0011 = decrease velocity  0100= velocity=0  0101= increase velocity  0110= reached upper bound  0111= reached lower bound  1000= writed the position on the pipe  1001=stop signal received  1010= reset signal received  1011= closure signal received.    the log number with an e in front means the relative operation failed");

    // condition for the signal:
    if (signal(SIGUSR1, sig_handler) == SIG_ERR) { // check if there is any stop signal
        perror("error receiving the signal from command_console"); // checking errors
        logger(log_pn_motor2, "e1001"); // write a error log message
    }

    if (signal(SIGUSR2, sig_handler) == SIG_ERR) { // check if there is any reset signal
        perror("error receiving the signal from command_console"); // checking errors
        logger(log_pn_motor2, "e1010"); // write a error log message
    }

    if (signal(SIGTERM, sig_handler) == SIG_ERR) { // check if there is any closure signal
        perror("error receiving the closure signal from the master in motor2"); // checking errors
        logger(log_pn_motor2, "e1011"); // write a error log message
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
        
        // read the pipe and compute the position along z
        r_Vz_m2 = read(Vz_m2, Vz_rcv_p, 1); // reading the pipe Vz
        if(r_Vz_m2 < 0 && errno != EAGAIN) {
            perror("error reading the pipe Vz from m2"); // checking errors
            z_i = z_i + (Vz_i * T);
            logger(log_pn_motor2, "e0010"); // write a error log message
            
        }
        else if (r_Vz_m2 > 0){ 
            if (Vz_rcv[0] == 0) { // decrease the velocity
                Vz_i = Vz_i - 0.25;
                z_i = z_i + (Vz_i * T);
                logger(log_pn_motor2, "0011"); // write a log message
            }

            else if (Vz_rcv[0] == 1) { // set velocity equal to 0
                Vz_i = 0;
                logger(log_pn_motor2, "0100"); // write a log message
            }

            else if (Vz_rcv[0] == 2) { // increase the velocity
                Vz_i = Vz_i + 0.25;
                z_i = z_i + (Vz_i * T);
                logger(log_pn_motor2, "0101"); // write a log message
            }
        }
        else {
            z_i = z_i + (Vz_i * T);
            logger(log_pn_motor2, "0010"); // write a error log message
        }
        
        // constrol if z reached the upper bound:
        if (z_i > 10) { // reached upper bound stop at that position
            z_i = 10;
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
        z_snd[0] = z_i;
        w_z_m2 = write(z_m2, z_snd_p, 4); // writing the position on the pipe
        if(w_z_m2 <= 0) { 
            perror("error tring to write on the z pipe from m2"); // checking errors
            logger(log_pn_motor2, "e1000"); // write a error log message
        }
        else if (w_z_m2 > 0){
            logger(log_pn_motor2, "1000"); // write a log message
        }
        memset(Vz_rcv_p, 0, sizeof(Vz_rcv)); // clear the receiving messages array
    }
}