#include "./../include/processB_utilities.h"
#include <bmpfile.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>

#define SEMAPHORE1 "/semaphore1"
#define SEMAPHORE2 "/semaphore2"

// declaring some variables for the shared memory:
const char * shm = "/shm"; // initialize the pathname of the shared memory
int shm_fd; // declare the file descriptor of the shared memory
int * shm_ptr; // declare the pointer to the shared memory
size_t shm_size = 1600 * 600; // dimension of the shared memory

// declaring some variables for the semphores:
sem_t * sem1; // declaring the semaphore 1 adress
int sem1_r; // declare the returned valeu of the wait function on semaphore 1
sem_t * sem2; // declaring the semaphore 2 adress
int sem2_r; // declare the returned valeu of the post function on semaphore 2

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

void sig_handler (int signo) {
    if (signo == SIGTERM) { // closure signal received
        if (munmap(shm_ptr, shm_size) < 0) { // unmap the shared memory
        perror("error unmapping the shared memory in processB"); // checking errors
        }
        if (shm_unlink(shm) < 0) { // unlink the shared memory
        perror("error unlinking the shared memory in processB"); // checking errors
        }
        if (sem_close(sem1) < 0) { // close the semaphore1
        perror("error closing the semaphore1 in processB"); // checking errors
        }
        if (sem_unlink(SEMAPHORE1) < 0) { // destroing the semaphore1
        perror("error unlinking the semaphore1 in processB"); // checking errors
        }
        if (sem_close(sem2) < 0) { // close the semaphore1
        perror("error closing the semaphore2 in processB"); // checking errors
        }
        if (sem_unlink(SEMAPHORE2) < 0) { // destroing the semaphore1
        perror("error unlinking the semaphore2 in processB"); // checking errors
        }
        if (raise(SIGKILL) != 0) { // proces commit suicide
            perror("error suiciding the processB"); // checking errors
        }
    }
}

void read_shm(int shm_size, int * shm_ptr, int * arr_ptr) {
    int i_max = shm_size; // initializing the dimension of the shared memory array
    int counter = 0; // initialize the counter
    for (int i = 1; i <= i_max; i++) { // for all the shared memory array elemnets
        arr_ptr[i] = shm_ptr[i]; // copy them in an array
    }
}
int main(int argc, char const *argv[]) {
    // logger variable
    const char * log_pn_processB = "./bin/log_files/processB.txt"; // initialize the log file path name
    remove(log_pn_processB); // remove the old log file

    // declare some internal variable:
    int image_arr[shm_size]; // initialize the vector that will hold the image
    int * image_arr_ptr = &image_arr[0]; // initialize the pointer to the array
    int c_counter = 0; // intialize the counter used to find the circle center 
    int xc; // declare the position of the center of the circle along x
    int yc; // declare the position of the center of the circle along y
    int history[80][30] = {{0}}; // initialize the array where i will store the history of ythe circle centers
    
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    logger(log_pn_processB, "log legend: 0001= created a blanck picture for the size   0010= opened the shared memory   0011= mapped the shared memory   0100= opened the semaphore1   0101= opened the semaphore 2   0110= decremented the semaphore2   0111= found the circle center   1000= incremented the semaphore 1   1001= printed the window   1010= signal received to close the process"); // write a log message
    
    // signal menagement to close the processes
    if (signal(SIGTERM, sig_handler) == SIG_ERR) { // check if there is any closure signal
        perror("error receiving the closure signal from the processA in processB"); // checking errors
        logger(log_pn_processB, "e1010"); // write a error log message
    }
    else {
        logger(log_pn_processB, "1010"); // write a error log message
    }
    
    logger(log_pn_processB, "0001"); // write a log message

    // Initialize UI
    init_console_ui();

    // open the shared memory:
    shm_fd = shm_open(shm, O_RDONLY | O_CREAT, 0666); // opening for read the shared memory
    if (shm_fd < 0) {
        perror("error opening the shared memory from processB"); // checking errors
        logger(log_pn_processB, "e0010"); // write a log message
    }
    else {
        logger(log_pn_processB, "0010"); // write a log message
        shm_ptr = (int *) mmap(NULL, shm_size, PROT_READ, MAP_SHARED, shm_fd, 0); // map the shared memory
        if (shm_ptr == MAP_FAILED) {
            perror("error mapping the shared memory from processB"); // checking errors
            logger(log_pn_processB, "e0011"); // write a log message
        }
        else {
            logger(log_pn_processB, "0011"); // write a log message
        }
    }

    /*bmp_header_t al = bmp_get_header(shm_ptr);
    int a = al.filesz;
    char arr2[5];
    sprintf(&arr2[0], "shm%i", a);
    logger(log_pn_processB, arr2);*/

    // open the semaphores:
    sem1 = sem_open(SEMAPHORE1, 0); // opening the semaphore1 with a starting valeu of 1
    if (sem1 < 0) {
        perror("error opening the semaphore1 from processB"); // checking errors
        logger(log_pn_processB, "e0100"); // write a log message
    }
    else {
        logger(log_pn_processB, "0100"); // write a log message
    }
    sem2 = sem_open(SEMAPHORE2, 0); // opening the semaphore2 with a starting valeu of 0
    if (sem2 < 0) {
        perror("error opening the semaphore2 from processB"); // checking errors
        logger(log_pn_processB, "e0101"); // write a log message
    }
    else {
        logger(log_pn_processB, "0101"); // write a log message
    }
    
    sleep(1);


    // Infinite loop
    while (TRUE) {

        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        else {
            int sem2_check1;
            int sem2_check2;
            int sem1_check1;
            int sem1_check2;
            //sem_getvalue(sem2, &sem2_check1);
            sem2_r = sem_wait(sem2); // get the exclusive access to the shared memory
            if (sem2_r < 0) {
                perror("error in the wait function on the semaphore 2 in the processB"); // checking errors
                logger(log_pn_processB, "e0110"); // write a log message
                sem_getvalue(sem2, &sem2_check2);
                sem_getvalue(sem1, &sem1_check1);
            }
            else {
                sem_getvalue(sem2, &sem2_check2);
                logger(log_pn_processB, "0110"); // write a log message
                mvaddch(10, 10, '+'); // print a plus in the window
                read_shm(shm_size, shm_ptr, image_arr_ptr); // read the shared memory
                for (int i = 0; i <= sizeof(image_arr_ptr); i++) {
                    if (image_arr[i] > 0) {
                        c_counter ++; 
                    }
                    else {
                        c_counter = 0;
                    }

                    if (c_counter >= 59) {
                        xc =((i + 1) - (round((i + 1) / 1600) * 1600) - 70) / 20;
                        yc = (round((i + 1) / 1600) - 10) / 20;
                        history[xc][yc] = 1;
                        goto found;
                    }
                }
                found: logger(log_pn_processB,"found"); // write a log message
                for (int i = 1; i <= 80; i++) {
                    for ( int j = 1; j <= 30; j++) {
                        if (history[i][j] == 1) {
                            mvaddch(i, j, '+');
                        }
                    }
                }
                /*int a = i_max;
                int b = j_max;
                char arr2[5];
                sprintf(&arr2[0], "i%ij%i", a, b);
                logger(log_pn_processB, arr2);
                int message[5];
                for (int i = 0; i <= 4; i++) {
                    message[i] = shm_ptr[i];
                }
                char arr2[5];
                char * arr2_ptr = &arr2[0];
                
                sprintf(arr2_ptr, "%i%i%i%i", message[0], message[1], message[2], message[3]);
                
                logger(log_pn_processB, arr2);*/
                
                sem_getvalue(sem1, &sem1_check1);
                sem1_r = sem_post(sem1); // notify processA that i've completed the work and he can read
                if (sem1_r < 0) {
                    perror("error in the post function on the semaphore 1 in the processB"); // checking errors
                    logger(log_pn_processB, "e1000"); // write a log message
                }
                else {
                    logger(log_pn_processB, "1000"); // write a log message
                }
            }

            sem_getvalue(sem1, &sem1_check2);
            
            char arr[13];
            sprintf(&arr[0], "21%i22%i11%i12%i", sem2_check1, sem2_check2, sem1_check1, sem1_check2);
            logger(log_pn_processB, arr); // write a log message

            
            logger(log_pn_processB, "1001"); // write a log message
            refresh();
            sleep(1);
        }
    }

    endwin();
    return 0;
}