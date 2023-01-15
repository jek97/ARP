#include "./../include/processA_utilities.h"
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

void clear_picture_shm(bmpfile_t *bmp) { // function to clear a picture (all the picture white)
    rgb_pixel_t color = {255, 255, 255, 255}; // define the color white
    int i_max = bmp_get_width(bmp); // number of row of the picture
    int j_max = bmp_get_height(bmp); // number of column of the picture
    for (int i = 1; i <= i_max; i++) { // for all the image points
        for (int j = 1; j <= j_max; j++) {
            bmp_set_pixel(bmp, i, j, color); // color the pixels of white color
        }
    }
}

void draw_circle_shm(bmpfile_t *bmp, int xc, int yc, int r, rgb_pixel_t color) { // function to draw a circle in the picture
    int i_max = bmp_get_width(bmp); // initialize the number of row of the picture
    int j_max = bmp_get_height(bmp); // initialize the number of column of the picture
    for (int i = 1; i <= i_max; i++) { // for all the image points
        for (int j = 1; j <= j_max; j++) {
            float d = sqrt(((i-xc)*(i-xc))+((j-yc)*(j-yc))); // compute the distance from the circle center
            if (d <= r) { // if the distance is less than or equal to the radius
                bmp_set_pixel(bmp, i, j, color); // color the pixel
            }
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


int main(int argc, char *argv[]) {
    // loger variable
    const char * log_pn_processA = "./bin/log_files/processA.txt"; // initialize the log file path name
    remove(log_pn_processA); // remove the old log file

    // declaring some variables for the shared memory:
    const char * shm = "/shm"; // initialize the pathname of the shared memory
    int shm_fd; // declare the file descriptor of the shared memory
    int ft; // declare the returned valeu of the function ftruncate
    int * shm_ptr; // declare the pointer to the shared memory
    int shm_size;

    // declaring some variables for the semphores:
    sem_t * sem1; // declaring the semaphore 1 adress
    //const char * semaphore1 = "/semaphore1"; // initialize the pathname of the semaphore1
    int sem1_r; // declare the returned valeu of the wait function on semaphore 1
    sem_t * sem2; // declaring the semaphore 2 adress
    //const char * semaphore2 = "/semaphore2"; // initialize the pathname of the semaphore2
    int sem2_r; // declare the returned valeu of the post function on semaphore 2
    
    // declare some variable to save the shared memory status:
    const char * shm_snapshot = "./out/snapshot.bmp"; // pathname of where to save the snapshot
    int w_s_out; // returned valeu of the write function on the pipe s
    int s_snd[] = {1}; // declare the s signal sending buffer
    int * s_snd_p = &s_snd[0]; // initialize the pointer to the s_snd array
    
    // declare some internal variable:
    int xc_w; // declare the x coordinate of the center of the circle in the ncurse window
    int yc_w; // declare the y coordinate of the center of the circle in the ncurse window
    int xc_shm; // declare the x coordinate of the center of the circle in the shared memory
    int yc_shm; // declare the y coordinate of the center of the circle in the shared memory
    int rc_shm = 30; // initialize the radius of the circle in the shared memory
    rgb_pixel_t c_color = {0, 0, 255, 255}; // initialize the circle color (BRGA) to red

    // declare some variable for the pipe between processA and the master
    int s_out; // declare the file descriptor of the pipe s
    const char *s = "./bin/named_pipes/s"; // initialize the the pipe s pathname

    logger(log_pn_processA, "log legend: 0001= created the blanck picture for the size   0010= opened the shared memory   0011= truncated the shared memory   0100= mapped the shared memory   0101= fullfilled the shared memory with an initial image   0110= opened the semaphore 1   0111= opened the semaphore 2   1000= saved a snapshot of the shared memory   1001= decremented the semaphore1   1010= drawed the new circle   1011= incremented the semaphore 2   1100= opened the s named pipe   1101= written the signal id on the pipe s   1110= initialized semaphore1   1111= initialized sempahore2   the log number with an e in front means the relative operation failed"); // write a log message

    // open the pipe:
    s_out = open(s, O_WRONLY); // open the pipe s to write on it
    if(s_out < 0){
        perror("error opening the pipe s from processA"); // checking errors
        logger(log_pn_processA, "e1100"); // write a log message
    }
    else {
        logger(log_pn_processA, "1100"); // write a log message
    }

    // create a blanck picture for its dimension used to initialize the shared memory:
    bmpfile_t *image; // Data structure for storing the bitmap file
    int width = 1600; // width of the image (in pixels)
    int height = 600; // Height of the image (in pixels)
    int depth = 4; // Depth of the image (1 for greyscale images, 4 for colored images)
    image = bmp_create(width, height, depth); // create the image
    shm_size = 6400; // set the shared memory dimension
    int pos = 0; // declaring the position in the array
    int i = 1;
    int i_max = width - 1; // initialize the number of row of the picture
    int j;
    int j_max = height - 1; // initialize the number of column of the picture
    rgb_pixel_t *p; // declaring the pixel variable
    int row[width];
    logger(log_pn_processA, "0001"); // write a log message

    // create the shared memory:
    shm_fd = shm_open(shm, O_RDWR | O_CREAT, 0666); // opening/creating the shared memory
    if (shm_fd < 0) {
        perror("error opening the shared memory from processA"); // checking errors
        logger(log_pn_processA, "e0010"); // write a log message
    }
    else {
        logger(log_pn_processA, "0010"); // write a log message
        ft = ftruncate(shm_fd, shm_size); // truncate the shared memory to the needed dimension
        if (ft == -1) {
            perror("error troncating the shared memory in processA"); // checking errors
            logger(log_pn_processA, "e0011"); // write a log message
        }
        else {
            logger(log_pn_processA, "0011"); // write a log message
            shm_ptr = (int *) mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // map the shared memory
            if (shm_ptr == MAP_FAILED) {
                perror("error mapping the shared memory from processA"); // checking errors
                logger(log_pn_processA, "e0100"); // write a log message
            }
            else {
                logger(log_pn_processA, "0100"); // write a log message
            }
        }
    }
    
    // paint the initial image:
    clear_picture_shm(image); // color the pcture in white
    logger(log_pn_processA, "0101"); // write a log message

    // create the semaphores:
    sem1 = sem_open(SEMAPHORE1, O_CREAT , S_IWUSR | S_IRUSR, 1); // create the semaphore1 with a starting valeu of 1
    if (sem1 < 0) {
        perror("error opening the semaphore1 from processA"); // checking errors
        logger(log_pn_processA, "e0110"); // write a log message
    }
    else {
        logger(log_pn_processA, "0110"); // write a log message
    }
    sem2 = sem_open(SEMAPHORE2, O_CREAT , S_IWUSR | S_IRUSR, 1); // create the semaphore2 with a starting valeu of 0
    if (sem2 < 0) {
        perror("error opening the semaphore2 from processA"); // checking errors
        logger(log_pn_processA, "e0111"); // write a log message
    }
    else {
        logger(log_pn_processA, "0111"); // write a log message
    }

    if (sem_init(sem1, 1, 1) < 0) {
        perror("error initializing the semaphore1 in processA");
        logger(log_pn_processA, "e1110"); // write a log message
    }
    else {
        logger(log_pn_processA, "1110"); // write a log message
    }
    if (sem_init(sem2, 1, 0) < 0) {
        perror("error initializing the semaphore2 in processA");
        logger(log_pn_processA, "e1111"); // write a log message
    }
    else {
        logger(log_pn_processA, "1111"); // write a log message
    }
    
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Infinite loop
    while (TRUE) {
        mvprintw(LINES - 1, 1, "Press end to exit");
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

        else if (cmd == KEY_END) {
            mvprintw(LINES - 1, 1, "closing processes");
            w_s_out = write(s_out, s_snd_p, 1); // writing the signal id on the pipe
            if(w_s_out <= 0) { 
                perror("error tring to write on the s pipe from porcessA"); // checking errors
                logger(log_pn_processA, "e1101"); // write a error log message
            }
            else {
                logger(log_pn_processA, "1101"); // write a error log message
            }

            if (munmap(shm_ptr, shm_size) < 0) { // unmap the shared memory
            perror("error unmapping the shared memory in processA"); // checking errors
            }
            if (shm_unlink(shm) < 0) { // unlink the shared memory
            perror("error unlinking the shared memory in processA"); // checking errors
            }
            if (sem_close(sem1) < 0) { // close the semaphore1
            perror("error closing the semaphore1 in processA"); // checking errors
            }
            if (sem_unlink(SEMAPHORE1) < 0) { // destroing the semaphore1
            perror("error unlinking the semaphore1 in processA"); // checking errors
            }
            if (sem_close(sem2) < 0) { // close the semaphore1
            perror("error closing the semaphore2 in processA"); // checking errors
            }
            if (sem_unlink(SEMAPHORE2) < 0) { // destroing the semaphore1
            perror("error unlinking the semaphore2 in processA"); // checking errors
            }
            if (raise(SIGKILL) != 0) { // proces commit suicide
                perror("error suiciding the processA"); // checking errors
            }
        }

        // Else, if user presses print button...
        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if(check_button_pressed(print_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Print button pressed");
                    refresh();
                    bmp_save(image, shm_snapshot);
                    logger(log_pn_processA, "1000"); // write a log message
                    sleep(1);
                    for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }

        // If input is an arrow key, move circle accordingly...
        else if(cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN) {
            // obtain the cricle center in the ncurse window
            xc_w = circle.x;
            yc_w = circle.y;
            // evaluate the circle center in the shared memory
            xc_shm = (20 * xc_w)+10;
            yc_shm = (20 * yc_w)+10;

            logger(log_pn_processA, "c"); // write a log message

            clear_picture_shm(image); // clear the shared memory picture from previous circles
            draw_circle_shm(image, xc_shm, yc_shm, rc_shm, c_color); // draw the new circle
            logger(log_pn_processA, "d"); // write a log message

            for (j = 0; j <= j_max; j++) {

                for (i = 0; i <= i_max; i++) {
                    p = bmp_get_pixel(image, i, j);
                    if (p->red == 255 && p->blue == 0 && p->green == 0) {
                        row[i] = 1;
                    }
                    else {
                        row[i] = 0;
                    }
                }
                
                int a = sizeof(row);
                char arr2[5];
                sprintf(&arr2[0], "row%i", a);
                logger(log_pn_processA, arr2);
                
                logger(log_pn_processA, "a"); // write a log message
                
                sem1_r = sem_wait(sem1); // get the exclusive access to the shared memory
                if (sem1_r < 0) {
                    perror("error in the wait function on the semaphore 1 in the processA"); // checking errors
                    logger(log_pn_processA, "e1001"); // write a log message
                }
                else {
                    logger(log_pn_processA, "1001"); // write a log message

                    for (i = 0; i <= i_max; i++) {
                        shm_ptr[i] = row[i];
                    }       
                    logger(log_pn_processA, "b"); // write a log message          
                
                    logger(log_pn_processA, "1010"); // write a log message
                    sem2_r = sem_post(sem2); // notify processB that i've completed the work and he can read
                    if (sem2_r < 0) {
                        perror("error in the post function on the semaphore 2 in the processA"); // checking errors
                        logger(log_pn_processA, "e1011"); // write a log message
                    }
                    else {
                        logger(log_pn_processA, "1011"); // write a log message
                    }
            }
            
            }
            move_circle(cmd); // move the circle in the ncurse window
            draw_circle(); // draw such
        }
    }
    
    endwin();
    return 0;
}