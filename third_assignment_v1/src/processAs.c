#include "./../include/processA_utilities.h"
#include <bmpfile.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
    const char * log_pn_processAs = "./bin/log_files/processAs.txt"; // initialize the log file path name

    // declaring some variables for the shared memory:
    const char * shm = "/shm"; // initialize the pathname of the shared memory
    int shm_fd; // declare the file descriptor of the shared memory
    int ft; // declare the returned valeu of the function ftruncate
    int * shm_ptr; // declare the pointer to the shared memory
    int shm_size;

    // declaring some variables for the semphores:
    sem_t * sem1; // declaring the semaphore 1 adress
    int sem1_r; // declare the returned valeu of the wait function on semaphore 1
    sem_t * sem2; // declaring the semaphore 2 adress
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
    bmpfile_t *image; // declare the Data structure for storing the bitmap file
    int width = 1600; // initialize the width of the image (in pixels)
    int height = 600; // initialize the height of the image (in pixels)
    int depth = 4; // initialize the Depth of the image (1 for greyscale images, 4 for colored images)
    int i; // declare the iteration variable 
    int i_max = width - 1; // initialize the number of row of the picture
    int j; // declare the iteration variable
    int j_max = height - 1; // initialize the number of column of the picture
    rgb_pixel_t *p; // declaring the pixel variable
    int row[width]; // declaring the array in which i will store the single line of the image
    shm_size = 6400; // set the shared memory dimension

    // declare some variable for the pipe between processA and the master
    int s_out; // declare the file descriptor of the pipe s
    const char *s = "./bin/named_pipes/s"; // initialize the the pipe s pathname

    // declare some variables for the socket:
    int sockfd, newsockfd, clilen, n; // declare the socket file descriptors, the size of the address of the client and the character readed and writed on the socket
    int portno = 5000; // Declare the port number (to decide)
    struct sockaddr_in serv_addr, cli_addr; // structure to store the server and client internet address
    int r_pA; // declare the returned variable of the read function on the socket
    char in_buf[5]; // declare the buffer in input
    int cmd_i; // declare the variable where i will store the command to move the circle

    remove(log_pn_processAs); // remove the old log file
    logger(log_pn_processAs, "log legend: 000001= opened the s pipe   000010= created the blanck picture   000011= opened the shared memory   000100= truncated the shared memory   000101= mapped the shared memory   000110= opened the semaphore1   000111= opened the semaphore 2   001000= initialized the semaphore1   001001= initialized the semaphore2   001010= opened the socket   001011= binded the socket to the port   001100= accepted the communication   001101= readed the socket   001110= window resized   001111= end button pressed   010000= closing message sent to the master   010001= closing the socket   010010= closed the pipe s   010011= unlinked the pipe s   010100= unmapped the shared memory   010101= unlinked the shared memory   010110= closed the semaphore1   010111= unlinked the semaphore1   011000= closed the semaphore2   011001= unlinked the semaphore2   011010= processA committed suicide   011011= saved a snapshot of the shared memory   011100= circle moved   011101= drawed the new circle   011110= filled the row array   011111= decremented the semaphore1   100000= sended the row   100001= incremented the sempahore2   the log number with an e in front means the relative operation failed"); // write a log message

    // open the pipe:
    s_out = open(s, O_WRONLY); // open the pipe s to write on it
    if(s_out < 0){
        perror("error opening the pipe s from processAs"); // checking errors
        logger(log_pn_processAs, "e000001"); // write a log message
    }
    else {
        logger(log_pn_processAs, "000001"); // write a log message
    }

    // create a blanck picture for its dimension used to initialize the shared memory:
    image = bmp_create(width, height, depth); // create the image
    clear_picture_shm(image); // color the pcture in white
    logger(log_pn_processAs, "000010"); // write a log message

    // create the shared memory:
    shm_fd = shm_open(shm, O_RDWR | O_CREAT, 0666); // opening/creating the shared memory
    if (shm_fd < 0) {
        perror("error opening the shared memory from processAs"); // checking errors
        logger(log_pn_processAs, "e000011"); // write a log message
    }
    else {
        logger(log_pn_processAs, "000011"); // write a log message
        ft = ftruncate(shm_fd, shm_size); // truncate the shared memory to the needed dimension
        if (ft == -1) {
            perror("error troncating the shared memory in processAs"); // checking errors
            logger(log_pn_processAs, "e000100"); // write a log message
        }
        else {
            logger(log_pn_processAs, "000100"); // write a log message
            shm_ptr = (int *) mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // map the shared memory
            if (shm_ptr == MAP_FAILED) {
                perror("error mapping the shared memory from processAs"); // checking errors
                logger(log_pn_processAs, "e000101"); // write a log message
            }
            else {
                logger(log_pn_processAs, "000101"); // write a log message
            }
        }
    }

    // create the semaphores:
    sem1 = sem_open(SEMAPHORE1, O_CREAT , S_IWUSR | S_IRUSR, 1); // create the semaphore1 with a starting valeu of 1
    if (sem1 < 0) {
        perror("error opening the semaphore1 from processAs"); // checking errors
        logger(log_pn_processAs, "e000110"); // write a log message
    }
    else {
        logger(log_pn_processAs, "000110"); // write a log message
    }
    sem2 = sem_open(SEMAPHORE2, O_CREAT , S_IWUSR | S_IRUSR, 1); // create the semaphore2 with a starting valeu of 0
    if (sem2 < 0) {
        perror("error opening the semaphore2 from processAs"); // checking errors
        logger(log_pn_processAs, "e000111"); // write a log message
    }
    else {
        logger(log_pn_processAs, "000111"); // write a log message
    }

    if (sem_init(sem1, 1, 1) < 0) {
        perror("error initializing the semaphore1 in processAs"); // checking errors
        logger(log_pn_processAs, "e001000"); // write a log message
    }
    else {
        logger(log_pn_processAs, "001000"); // write a log message
    }
    if (sem_init(sem2, 1, 0) < 0) {
        perror("error initializing the semaphore2 in processAs"); // checking errors
        logger(log_pn_processAs, "e001001"); // write a log message
    }
    else {
        logger(log_pn_processAs, "001001"); // write a log message
    }
    
    // create the socket:
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // open the socket
    if (sockfd < 0) {
        perror("error opening the socket in processAs"); // checking errors
        logger(log_pn_processAs, "e001010"); // write a log message
    }
    else {
        logger(log_pn_processAs, "001010"); // write a log message
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); // clean the struct in which i will store the host data
    serv_addr.sin_family = AF_INET; // fulfill the struct in which i will store the host data
    serv_addr.sin_addr.s_addr = INADDR_ANY; // fulfill the struct in which i will store the host data
    serv_addr.sin_port = htons(portno); // fulfill the struct in which i will store the host data
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { // binding the socket to the port
        perror("error binding the socket to the port in processAs"); // checking errors
        logger(log_pn_processAs, "e001011"); // write a log message
    }
    else {
        logger(log_pn_processAs, "001011"); // write a log message
    }

    listen(sockfd,5); // listening for possible connection
    clilen = sizeof(cli_addr); // initialize the struct field
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); // accept the communication
    if (newsockfd < 0) {
        perror("error accepting the communication in processAs"); // checking errors
        logger(log_pn_processAs, "e001100"); // write a log message
    }
    else {
        logger(log_pn_processAs, "001100"); // write a log message
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

        bzero(in_buf, sizeof(in_buf)); // clean the receiving buffer from previous data
        r_pA = read(newsockfd, in_buf, sizeof(in_buf)); // writing the signal id on the pipe
        if(r_pA <= 0) { 
            perror("error reading from the socket in porcessAs"); // checking errors
            logger(log_pn_processAs, "e001101"); // write a error log message
        }
        else {
            logger(log_pn_processAs, "001101"); // write a error log message
        }
        int in = 0;
        int a = 0;
        for (int i = 0; i <= sizeof(in_buf); i++) {
            if (in_buf[i] == '\0') {
                break;
            }
            else {
                a = in_buf[i] - '0';
                in = 10 * in + a;
            }
        }
    
        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            logger(log_pn_processAs, "001110"); // write a error log message
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        else if (in == KEY_END) {
            logger(log_pn_processAs, "001111"); // write a error log message
            
            w_s_out = write(s_out, s_snd_p, 1); // writing the signal id on the pipe
            if(w_s_out <= 0) { 
                perror("error tring to write on the s pipe from porcessAs"); // checking errors
                logger(log_pn_processAs, "e010000"); // write a error log message
            }
            else {
                logger(log_pn_processAs, "010000"); // write a error log message
            }

            if (close(newsockfd) < 0) { // closing the socket
                perror("error closing the socket in processAs"); // checking errors
                logger(log_pn_processAs, "e010001"); // write a error log message
            }
            else {
                logger(log_pn_processAs, "010001"); // write a error log message
            }

            if (close(s_out) < 0) { // closing the pipe s
                perror("error closing the pipe s from processAs"); // checking errors
                logger(log_pn_processAs, "e010010"); // write a error log message
            }
            else {
                logger(log_pn_processAs, "010010"); // write a error log message
            }
            
            if (unlink(s) < 0) { // unlinking the pipe s
                perror("error unlinking the pipe s form processAs"); // checking errors
                logger(log_pn_processAs, "e010011"); // write a error log message
            }
            else {
                logger(log_pn_processAs, "010011"); // write a error log message
            }
            
            if (munmap(shm_ptr, shm_size) < 0) { // unmap the shared memory
                perror("error unmapping the shared memory in processAs"); // checking errors
                logger(log_pn_processAs, "e010100"); // write a error log message
            }
            else {
                logger(log_pn_processAs, "010100"); // write a error log message
            }
            
            if (shm_unlink(shm) < 0) { // unlink the shared memory
                perror("error unlinking the shared memory in processAs"); // checking errors
                logger(log_pn_processAs, "e010101"); // write a error log message
            }
            else {
                logger(log_pn_processAs, "010101"); // write a error log message
            }
            
            if (sem_close(sem1) < 0) { // close the semaphore1
                perror("error closing the semaphore1 in processAs"); // checking errors
                logger(log_pn_processAs, "e010110"); // write a error log message
            }
            else {
                logger(log_pn_processAs, "010110"); // write a error log message
            }
            
            if (sem_unlink(SEMAPHORE1) < 0) { // destroing the semaphore1
                perror("error unlinking the semaphore1 in processAs"); // checking errors
                logger(log_pn_processAs, "e010111"); // write a error log message
            }
            else {
                logger(log_pn_processAs, "010111"); // write a error log message
            }
            
            if (sem_close(sem2) < 0) { // close the semaphore1
                perror("error closing the semaphore2 in processAs"); // checking errors
                logger(log_pn_processAs, "e011000"); // write a error log message
            }
            else {
                logger(log_pn_processAs, "011000"); // write a error log message
            }
            
            if (sem_unlink(SEMAPHORE2) < 0) { // destroing the semaphore1
                perror("error unlinking the semaphore2 in processAs"); // checking errors
                logger(log_pn_processAs, "e011001"); // write a error log message
            }
            else {
                logger(log_pn_processAs, "011001"); // write a error log message
            }
            if (raise(SIGKILL) != 0) { // proces commit suicide
                perror("error suiciding the processAs"); // checking errors
                logger(log_pn_processAs, "e011010"); // write a error log message
            }
            else {
                logger(log_pn_processAs, "011010"); // write a error log message
            }
        }

        // Else, if user presses print button...
        else if(in == KEY_PRINT) {
            mvprintw(LINES - 1, 1, "Print button pressed");
            refresh();
            bmp_save(image, shm_snapshot); // save a snapshot of the shared memory
            logger(log_pn_processAs, "011011"); // write a log message
            sleep(1);
            for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                mvaddch(LINES - 1, j, ' ');
            }            
        }

        // If input is an arrow key, move circle accordingly...
        else if(in == KEY_LEFT || in == KEY_RIGHT || in == KEY_UP || in == KEY_DOWN) {

            if (in == KEY_LEFT) { // check which arrow message arrived
                cmd_i = KEY_LEFT; // set the command variable
            }
            else if (in == KEY_RIGHT) { // check which arrow message arrived
                cmd_i = KEY_RIGHT; // set the command variable
            }
            else if (in == KEY_UP) { // check which arrow message arrived
                cmd_i = KEY_UP; // set the command variable
            }
            else if (in == KEY_DOWN) { // check which arrow message arrived
                cmd_i = KEY_DOWN; // set the command variable
            }

            move_circle(cmd_i); // move the circle in the ncurse window
            draw_circle(); // draw such
            logger(log_pn_processAs, "011100"); // write a error log message

            // obtain the cricle center in the ncurse window
            xc_w = circle.x;
            yc_w = circle.y;

            // evaluate the circle center in the shared memory
            xc_shm = (20 * xc_w);
            yc_shm = (20 * yc_w);

            clear_picture_shm(image); // clear the shared memory picture from previous circles
            draw_circle_shm(image, xc_shm, yc_shm, rc_shm, c_color); // draw the new circle
            logger(log_pn_processAs, "011101"); // write a error log message

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
                logger(log_pn_processAs, "011110"); // write a error log message
                
                sem1_r = sem_wait(sem1); // get the exclusive access to the shared memory
                if (sem1_r < 0) {
                    perror("error in the wait function on the semaphore 1 in the processAs"); // checking errors
                    logger(log_pn_processAs, "e011111"); // write a log message
                }
                else {
                    logger(log_pn_processAs, "011111"); // write a log message

                    for (i = 0; i <= i_max; i++) {
                        shm_ptr[i] = row[i];
                    } 
                    logger(log_pn_processAs, "100000"); // write a log message

                    sem2_r = sem_post(sem2); // notify processB that i've completed the work and he can read
                    if (sem2_r < 0) {
                        perror("error in the post function on the semaphore 2 in the processAs"); // checking errors
                        logger(log_pn_processAs, "e100001"); // write a log message
                    }
                    else {
                        logger(log_pn_processAs, "100001"); // write a log message
                    }
                }
            }
        }
    }
    
    endwin();
    return 0;
}