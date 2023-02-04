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
#include <sys/types.h>
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
    const char * log_pn_processA = "./bin/log_files/processA.txt"; // initialize the log file path name

    // declare some variables for the socket:
    int sockfd, newsockfd, clilen, n; // declare the socket file descriptors, the size of the address of the client and the character readed and writed on the socket
    int portno = 0; // Declare the port number (to decide)
    struct sockaddr_in serv_addr, cli_addr; // structure to store the server and client internet address
    char out_buf[2]; // declare the buffer in output
    int w_pA; // declare the returned valeu of the write function on the socket

    remove(log_pn_processA); // remove the old log file
    logger(log_pn_processA, "log legend: 000001= opened the s pipe   000010= created the blanck picture   000011= opened the shared memory   000100= truncated the shared memory   000101= mapped the shared memory   000110= opened the semaphore1   000111= opened the semaphore 2   001000= initialized the semaphore1   001001= initialized the semaphore2   001010= window resized   001011= end button pressed   001100= closing message sent to the master   001101= closed the pipe s   001110= unlinked the pipe s   001111= unmapped the shared memory   010000= unlinked the shared memory   010001= closed the semaphore1   010010= unlinked the semaphore1   010011= closed the semaphore2   010100= unlinked the semaphore2   010101= processA committed suicide   010110= saved a snapshot of the shared memory   010111= circle moved   011000= drawed the new circle   011001= filled the row array   011010= decremented the semaphore1   011011= sended the row   011100= incremented the sempahore2   011101= opened the socket   011110= binding the socket   011111= socket connection enstablished   100000= write the end key on the socket   the log number with an e in front means the relative operation failed"); // write a log message

    // create the socket:
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // open the socket
    if (sockfd < 0) {
        perror("error opening the socket from processA"); // checking errors
        logger(log_pn_processA, "e011101"); // write a log message
    }
    else {
        logger(log_pn_processA, "011101"); // write a log message
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); // set all the values of the server address buffer equal to zero
    serv_addr.sin_family = AF_INET; // set the address family
    serv_addr.sin_port = htons(portno); // set the port number
    serv_addr.sin_addr.s_addr = INADDR_ANY; // set the IP address of the host
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { // binding the socket to the port
        perror("error binding the socket from processA"); // checking errors
        logger(log_pn_processA, "e011110"); // write a log message
    }
    else {
        logger(log_pn_processA, "011110"); // write a log message
    }

    listen(sockfd, 5); // listen for connections (max 5)
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, clilen); // enstablish the connection with the client

    if (newsockfd < 0) {
        perror("error waiting from connections in processA"); // checking errors
        logger(log_pn_processA, "e011111"); // write a log message
    }
    else {
        logger(log_pn_processA, "011111"); // write a log message
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
            logger(log_pn_processA, "001010"); // write a error log message
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        else if (cmd == KEY_END) {
            logger(log_pn_processA, "001011"); // write a error log message
            sprintf(out_buf, "%i", KEY_END);
            w_pA = write(newsockfd, out_buf, sizeof(out_buf)); // write the related key on the socket
            if (w_pA <= 0) {
                perror("error tring to write on the socket the end command from porcessA"); // checking errors
                logger(log_pn_processA, "e100000"); // write a error log message
            }
            else {
                logger(log_pn_processA, "100000"); // write a error log message
            }

            if (raise(SIGKILL) != 0) { // proces commit suicide
                perror("error suiciding the processA"); // checking errors
                logger(log_pn_processA, "e010101"); // write a error log message
            }
            else {
                logger(log_pn_processA, "010101"); // write a error log message
            }
        }

        // Else, if user presses print button...
        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if(check_button_pressed(print_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Print button pressed");
                    refresh();
                    sprintf(out_buf, "p");
                    w_pA = write(newsockfd, out_buf, sizeof(out_buf)); // write the related message on the socket
                    if (w_pA <= 0) {
                        perror("error tring to write on the socket the print command from porcessA"); // checking errors
                        logger(log_pn_processA, "e100000"); // write a error log message
                    }
                    else {
                        logger(log_pn_processA, "100000"); // write a error log message
                    }
                    sleep(1);
                    for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }

        // If input is an arrow key, move circle accordingly...
        else if(cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN) {
            logger(log_pn_processA, "010111"); // write a error log message
            move_circle(cmd); // move the circle in the ncurse window
            draw_circle(); // draw such
            
            if (cmd == KEY_LEFT) {
                sprintf(out_buf, "%i", KEY_LEFT);
                w_pA = write(newsockfd, out_buf, sizeof(out_buf)); // write the related message on the socket
                if (w_pA <= 0) {
                    perror("error tring to write on the socket the left command from porcessA"); // checking errors
                    logger(log_pn_processA, "e100000"); // write a error log message
                }
                else {
                    logger(log_pn_processA, "100000"); // write a error log message
                }
            }
            else if (cmd == KEY_RIGHT) {
                sprintf(out_buf, "%i", KEY_RIGHT);
                w_pA = write(newsockfd, out_buf, sizeof(out_buf)); // write the related message on the socket
                if (w_pA <= 0) {
                    perror("error tring to write on the socket the right command from porcessA"); // checking errors
                    logger(log_pn_processA, "e100000"); // write a error log message
                }
                else {
                    logger(log_pn_processA, "100000"); // write a error log message
                }
            }
            else if (cmd == KEY_UP) {
                sprintf(out_buf, "%i", KEY_UP);
                w_pA = write(newsockfd, out_buf, sizeof(out_buf)); // write the related message on the socket
                if (w_pA <= 0) {
                    perror("error tring to write on the socket the up command from porcessA"); // checking errors
                    logger(log_pn_processA, "e100000"); // write a error log message
                }
                else {
                    logger(log_pn_processA, "100000"); // write a error log message
                }
            }
            else if (cmd == KEY_DOWN) {
                sprintf(out_buf, "%i", KEY_DOWN);
                w_pA = write(newsockfd, out_buf, sizeof(out_buf)); // write the related message on the socket
                if (w_pA <= 0) {
                    perror("error tring to write on the socket the down command from porcessA"); // checking errors
                    logger(log_pn_processA, "e100000"); // write a error log message
                }
                else {
                    logger(log_pn_processA, "100000"); // write a error log message
                }
            }
            
            
        }
    }
    
    endwin();
    return 0;
}