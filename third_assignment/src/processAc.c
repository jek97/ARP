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
#include <netdb.h> 

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
    const char * log_pn_processAc = "./bin/log_files/processAc.txt"; // initialize the log file path name

    // declare some variables for the socket:
    int sockfd, newsockfd, clilen, n; // declare the socket file descriptors, the size of the address of the client and the character readed and writed on the socket
    int portno = 50000; // Declare the port number (to decide)
    struct sockaddr_in serv_addr; // structure to store the server internet address
    struct hostent * server; // pointer to a struct containing the alias of the server
    const char * server_nam = "LazyMachine"; // name of the server
    char out_buf[2]; // declare the buffer in output
    int w_pA; // declare the returned valeu of the write function on the socket

    remove(log_pn_processAc); // remove the old log file
    logger(log_pn_processAc, "log legend:   0001= iopened the socket   0010= obtained the host informations   0011= connected to the server   0100= windows resized   0101= end command received   0110= written the end command on the socket   0111= closed the socket   1000= process committed suicide   1001= print command sended   1010= arrow command key received   1011= relative arrow command written on the socket   the log number with an e in front means the relative operation failed"); // write a log message

    // create the socket:
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // opening the socket
    if (sockfd < 0) {
        perror("error opening the socket in processAc"); // checking errors
        logger(log_pn_processAc, "e0001"); // write a log message
    }
    else {
        logger(log_pn_processAc, "0001"); // write a log message
    }

    server = gethostbyname(server_nam); // obtaining the host informations
    if (server == NULL) {
        perror("error obtaining the host informations in processAc"); // checking errors
        logger(log_pn_processAc, "e0010"); // write a log message
    }
    else {
        logger(log_pn_processAc, "0010"); // write a log message
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); // cleaning the struct where i will store the host information
    serv_addr.sin_family = AF_INET; // fulfill the struct with the host informations
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length); // fulfill the struct with the host informations
    serv_addr.sin_port = htons(portno); // fulfill the struct with the host informations

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) { // connecting to the server
        perror("error connecting to the server in processAs"); // checking errors
        logger(log_pn_processAc, "e0011"); // write a log message
    }
    else {
        logger(log_pn_processAc, "0011"); // write a log message
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
            logger(log_pn_processAc, "0100"); // write a log message
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        else if (cmd == KEY_END) {
            logger(log_pn_processAc, "0101"); // write a log message

            bzero(out_buf, sizeof(out_buf)); // clean the buffer where i will store the data received by the socket
            sprintf(out_buf, "E"); // fulfill the buffer with the new data
            w_pA = write(sockfd, &out_buf[0], sizeof(out_buf)); // writing the message on the socket
            if(w_pA <= 0) { 
                perror("error writing on the socket the E command from porcessAc"); // checking errors
                logger(log_pn_processAc, "e0110"); // write a log message
            }
            else {
                logger(log_pn_processAc, "0110"); // write a log message
            }

            if (close(sockfd) < 0) { // close the socket
                perror("error closing socket from processAc"); // checking errors
                logger(log_pn_processAc, "e0111"); // write a log message
            }
            else {
                logger(log_pn_processAc, "0111"); // write a error log message
            }
            
            if (raise(SIGKILL) != 0) { // proces commit suicide
                perror("error suiciding the processAc"); // checking errors
                logger(log_pn_processAc, "e1000"); // write a log message
            }
            else {
                logger(log_pn_processAc, "1000"); // write a log message
            }
        }

        // Else, if user presses print button...
        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if(check_button_pressed(print_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Print button pressed");
                    refresh();
                    
                    bzero(out_buf, sizeof(out_buf)); // clean the buffer where i will store the data received by the socket
                    sprintf(out_buf, "P"); // fulfill the buffer with the new data
                    w_pA = write(sockfd, &out_buf[0], sizeof(out_buf)); // writing the message on the socket
                    if(w_pA <= 0) { 
                        perror("error writing on the socket the command P from porcessAc"); // checking errors
                        logger(log_pn_processAc, "e1001"); // write a log message
                    }
                    else {
                        logger(log_pn_processAc, "1001"); // write a log message
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
            logger(log_pn_processAc, "1010"); // write a log message
            move_circle(cmd); // move the circle in the ncurse window
            draw_circle(); // draw such

            bzero(out_buf, sizeof(out_buf)); // clean the buffer where i will store the data received by the socket
            if (cmd == KEY_LEFT) { // checking which arrow key was pressed
                sprintf(out_buf, "L"); // fill the buffer in output
            }
            else if (cmd == KEY_RIGHT) { // checking which arrow key was pressed
                sprintf(out_buf, "R"); // fill the buffer in output
            }
            else if (cmd == KEY_UP) { // checking which arrow key was pressed
                sprintf(out_buf, "U"); // fill the buffer in output
            }
            else if (cmd == KEY_DOWN) { // checking which arrow key was pressed
                sprintf(out_buf, "D"); // fill the buffer in output
            }
            
            w_pA = write(sockfd, &out_buf[0], sizeof(out_buf)); // writing the message on the socket
            if(w_pA <= 0) { 
                perror("error writing the arrow command on the socket from porcessAc"); // checking errors
                logger(log_pn_processAc, "e1011"); // write a log message
            }
            else {
                logger(log_pn_processAc, "1011"); // write a log message
            }

        }
    }
    
    endwin();
    return 0;
}