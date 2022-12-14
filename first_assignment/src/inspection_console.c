#include "./../include/inspection_utilities.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int ic; // declaring control variable
int jc; // declaring control variable
int kc; // declaring control variable

int ic1; // declaring control variable
int jc1; // declaring control variable
int kc1; // declaring control variable

int x_ins_in; // declare the file descriptor of the pipe x_c
int z_ins_in; // declare the file descriptor of the pipe z_c
char *x_c = "/named_pipes/x_c"; // initialize the pipe x_c pathname
char *z_c = "/named_pipes/z_c"; // initialize the pipe z_c pathname

int pid_cmd_dim_fd; // declare the file descriptor of the pipe pid_cmd_dim
int pid_m1_dim_fd; // declare the file descriptor of the pipe pid_m1_dim
int pid_m2_dim_fd; // declare the file descriptor of the pipe pid_m2_dim
char *pid_cmd_dim_name = "/named_pipes/pid_cmd_dim"; // initialize the pipe pid_cmd_dim pathname
char *pid_m1_dim_name = "/named_pipes/pid_m1_dim"; // initialize the pipe pid_m1_dim pathname
char *pid_m2_dim_name = "/named_pipes/pid_m2_dim"; // initialize the pipe pid_m2_dim pathname

int pid_cmd_fd; // declare the file descriptor of the pipe pid_cmd
int pid_m1_fd; // declare the file descriptor of the pipe pid_m1
int pid_m2_fd; // declare the file descriptor of the pipe pid_m2
char *pid_cmd_name = "/named_pipes/pid_cmd"; // initialize the pipe pid_cmd pathname
char *pid_m1_name = "/named_pipes/pid_m1"; // initialize the pipe pid_m1 pathname
char *pid_m2_name = "/named_pipes/pid_m2"; // initialize the pipe pid_m2 pathname

int x_rcv[1]; // declare the x position receiving buffer
int z_rcv[1]; // declare the z position receiving buffer
int pid_cmd_dim_rcv[]; // declare the cmd pid dimension receiving buffer
int pid_m1_dim_rcv[]; // declare the m1 pid dimension receiving buffer
int pid_m2_dim_rcv[]; // declare the m2 pid dimension receiving buffer

//int pid_cmd_rcv[]; // declare the cmd pid receiving buffer
//int pid_m1_rcv[]; // declare the m1 pid receiving buffer
//int pid_m2_rcv[]; // declare the m2 pid receiving buffer

fd_set rfds; // declare the select mode
struct timeval tv; // declare the time interval of the select function
int retvale; // declare the returned valeu
int nfds = 2; // initialize number of fd starting from 0
int fd; // declare the counter for FD_ISSET

fd_set rfds1; // declare the select mode
struct timeval tv1; // declare the time interval of the select function
int retvale1; // declare the returned valeu
int nfds1 = 2; // initialize number of fd starting from 0
int fd1; // declare the counter for FD_ISSET

fd_set rfds2; // declare the select mode
struct timeval tv2; // declare the time interval of the select function
int retvale2; // declare the returned valeu
int nfds2 = 1; // initialize number of fd starting from 0
int fd2; // declare the counter for FD_ISSET

float x, z; // End-effector coordinates

int pid_cmd; // declare the variable where i will store the pid of the cmd
int pid_m1; // declare the variable where i will store the pid of the m1
int pid_m2; // declare the variable where i will store the pid of the m2

int main(int argc, char const *argv[]){
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // open the pipes:
    // input pipes
    x_ins_in = open(x_c, O_RDONLY); // open the pipe x_c to read on it
    if(x_ins_in < 0){
        perror("error opening the pipe x_c from inspection"); // checking errors
    }

    z_ins_in = open(z_c, O_RDONLY); // open the pipe z_c to read on it
    if(z_ins_in < 0){
        perror("error opening the pipe z_c from inspection"); // checking errors
    }

    // pid dimension communication pipes:
    pid_cmd_dim_fd = open(pid_cmd_dim_name, O_RDONLY); // open the pipe pid_cmd_dim to read from it
    if(pid_cmd_dim_fd < 0){
        perror("error opening the pipe pid_cmd_dim from inspection"); // checking errors
    }

    pid_m1_dim_fd = open(pid_m1_dim_name, O_RDONLY); // open the pipe pid_m1_dim to read from it
    if(pid_m1_dim_fd < 0){
        perror("error opening the pipe pid_m1_dim from inspection"); // checking errors
    }

    pid_m2_dim_fd = open(pid_m2_dim_name, O_RDONLY); // open the pipe pid_m2_dim to read from it
    if(pid_m2_dim_fd < 0){
        perror("error opening the pipe pid_m2_dim from inspection"); // checking errors
    }

    // pid communication pipes:
    pid_cmd_fd = open(pid_cmd_name, O_RDONLY); // open the pipe pid_cmd to read from it
    if(pid_cmd_fd < 0){
        perror("error opening the pipe pid_cmd from inspection"); // checking errors
    }

    pid_m1_fd = open(pid_m1_name, O_RDONLY); // open the pipe pid_m1 to read from it
    if(pid_m1_fd < 0){
        perror("error opening the pipe pid_m1 from inspection"); // checking errors
    }

    pid_m2_fd = open(pid_m2_name, O_RDONLY); // open the pipe pid_m2 to read from it
    if(pid_m2_fd < 0){
        perror("error opening the pipe pid_m2 from inspection"); // checking errors
    }

    // Initialize User Interface 
    init_console_ui();

    // loop to obtain the pid dimension:
    while((ic+jc+kc)<3){
        // setting up the select
        FD_ZERO(&rfds); // clear all the fd in the set
        FD_SET(pid_cmd_dim_fd, &rfds); // put the fd of pid_cmd_dim_fd in the set
        FD_SET(pid_m1_dim_fd, &rfds); // put the fd of pid_m1_dim_fd in the set
        FD_SET(pid_m2_dim_fd, &rfds); // put the fd of pid_m2_dim_fd in the set
        tv.tv_sec = 1; // set the time interval in seconds
        tv.tv_usec = 0; // set the time interval in microseconds

        if (select((nfds+1), &rfds, NULL, NULL, &tv) < 0) { // checking errors
            perror("error in the pid dimension select of inspect");
        }
        else if (select((nfds+1), &rfds, NULL, NULL, &tv) == 0){
            perror("timer elapsed in inspect pid dimension select without data");
        }
        else{
            for (fd = 0; fd <= nfds; fd++) {
                if (FD_ISSET(fd, &rfds)) { // checking if there is a pipe readyfor communications
                    if (fd == pid_cmd_dim_fd) { // the pipe pid_cmd_dim_fd is ready for communicate
                        if(read(pid_cmd_dim_fd, pid_cmd_dim_rcv, sizeof(pid_cmd_dim_rcv)) < 0) { // checking errors
                            perror("error reading the pipe pid_cmd_dim from inspect"); 
                        }
                        else if (read(pid_cmd_dim_fd, pid_cmd_dim_rcv, sizeof(pid_cmd_dim_rcv)) > 0) { // otherwise read the pid dimension of cmd
                            int pid_cmd_rcv[pid_cmd_dim_rcv[0]]; // declare the cmd pid receiving buffer, look at the end of the program
                            ic = 1;
                        }
                    }
                    else if (fd == pid_m1_dim_fd) { // the pipe pid_m1_dim_fd is ready for communicate
                        if(read(pid_m1_dim_fd, pid_m1_dim_rcv, sizeof(pid_m1_dim_rcv)) < 0) { // checking errors
                            perror("error reading the pipe pid_m1_dim from inspect"); 
                        }
                        else if (read(pid_m1_dim_fd, pid_m1_dim_rcv, sizeof(pid_m1_dim_rcv)) > 0) { // otherwise read the pid dimension of m1
                            int pid_m1_rcv[pid_m1_dim_rcv[0]]; // declare the m1 pid receiving buffer, look at the end of the program
                            jc = 1;
                        }
                    }
                    else if (fd == pid_m2_dim_fd) { // the pipe pid_m2_dim_fd is ready for communicate
                        if(read(pid_m2_dim_fd, pid_m2_dim_rcv, sizeof(pid_m2_dim_rcv)) < 0) { // checking errors
                            perror("error reading the pipe pid_m2_dim from inspect"); 
                        }
                        else if (read(pid_m2_dim_fd, pid_m2_dim_rcv, sizeof(pid_m2_dim_rcv)) > 0) { // otherwise read the pid dimension of m2
                            int pid_m2_rcv[pid_m2_dim_rcv[0]]; // declare the m2 pid receiving buffer, look at the end of the program
                            kc = 1;
                        }
                    }
                }
            }
    }

    // loop to set the pid of the processes:
    while((ic1+jc1+kc1)<3){
        // setting up the select
        FD_ZERO(&rfds1); // clear all the fd in the set
        FD_SET(pid_cmd_fd, &rfds1); // put the fd of pid_cmd_fd in the set
        FD_SET(pid_m1_fd, &rfds1); // put the fd of pid_m1_fd in the set
        FD_SET(pid_m2_fd, &rfds1); // put the fd of pid_m2_fd in the set
        tv.tv_sec = 1; // set the time interval in seconds
        tv.tv_usec = 0; // set the time interval in microseconds

        if (select((nfds1+1), &rfds1, NULL, NULL, &tv1) < 0) { // checking errors
            perror("error in the pid select of inspect");
        }
        else if (select((nfds1+1), &rfds1, NULL, NULL, &tv1) == 0){
            perror("timer elapsed in inspect pid select without data");
        }
        else{
            for (fd1 = 0; fd1 <= nfds1; fd1++) {
                if (FD_ISSET(fd1, &rfds1)) { // checking if there is a pipe ready for communications
                    if (fd1 == pid_cmd_fd) { // the pipe pid_cmd_fd is ready for communicate
                        if(read(pid_cmd_fd, pid_cmd_rcv, sizeof(pid_cmd_rcv)) < 0) { // checking errors
                            perror("error reading the pipe pid_cmd from inspect"); 
                        }
                        else if (read(pid_cmd_fd, pid_cmd_rcv, sizeof(pid_cmd_rcv)) > 0) { // otherwise read the pid of cmd
                            pid_cmd = pid_cmd_rcv[]; // probably here i will need to concatenate the array elements
                            ic = 1;
                        }
                    }
                    else if (fd1 == pid_m1_fd) { // the pipe pid_m1_fd is ready for communicate
                        if(read(pid_m1_fd, pid_m1_rcv, sizeof(pid_m1_rcv)) < 0) { // checking errors
                            perror("error reading the pipe pid_m1 from inspect"); 
                        }
                        else if (read(pid_m1_fd, pid_m1_rcv, sizeof(pid_m1_rcv)) > 0) { // otherwise read the pid of m1
                            pid_m1 = pid_m1_rcv[]; // probably here i will need to concatenate the array elements
                            jc = 1;
                        }
                    }
                    else if (fd1 == pid_m2_fd) { // the pipe pid_m2_fd is ready for communicate
                        if(read(pid_m2_fd, pid_m2_rcv, sizeof(pid_m2_rcv)) < 0) { // checking errors
                            perror("error reading the pipe pid_m2 from inspect"); 
                        }
                        else if (read(pid_m2_fd, pid_m2_rcv, sizeof(pid_m2_rcv)) > 0) { // otherwise read the pid of m2
                            pid_m2 = pid_m2_rcv[]; // probably here i will need to concatenate the array elements
                            kc = 1;
                        }
                    }
                }
            }
    }

    // Infinite loop
    while(TRUE)	{

        // setting up the select
        FD_ZERO(&rfds2); // clear all the fd in the set
        FD_SET(x_ins_in, &rfds2); // put the fd of x_ins_in in the set
        FD_SET(z_ins_in, &rfds2); // put the fd of z_ins_in in the set
        tv2.tv2_sec = 5; // set the time interval in seconds
        tv2.tv2_usec = 0; // set the time interval in microseconds

        if (select((nfds2+1), &rfds2, NULL, NULL, &tv2) < 0) { // checking errors
            perror("error in the select of inspect");
        }
        else if (select((nfds2+1), &rfds2, NULL, NULL, &tv2) == 0){
            perror("timer elapsed in inspect select without data");
        }
        else{
            for (fd2 = 0; fd2 <= nfds2; fd2++) {
                if (FD_ISSET(fd2, &rfds2)) { // checking if there is a pipe readyfor communications
                    if (fd2 == x_ins_in) { // the pipe x_ins_in is ready for communicate
                        if(read(x_ins_in, x_rcv, sizeof(x_rcv)) < 0) { // checking errors
                            perror("error reading the pipe x_c from inspect"); 
                        }
                        else if (read(x_ins_in, x_rcv, sizeof(x_rcv)) > 0) { // otherwise read the position x
                            x = x_rcv[0];
                        }
                    }
                    else if (fd2 == z_ins_in) { // the pipe z_ins_in is ready for communicate
                        if(read(z_ins_in, z_rcv, sizeof(z_rcv)) < 0) { // checking errors
                            perror("error reading the pipe z_c from inspect"); 
                        }
                        else if (read(z_ins_in, z_rcv, sizeof(z_rcv)) > 0) { // otherwise read the position z
                            z = z_rcv[0];
                        }
                    }
                }
            }
        }

        // Get mouse/resize commands in non-blocking mode...
        int cmd = getch();

        // If user resizes screen, re-draw UI
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }
        // Else if mouse has been pressed
        else if(cmd == KEY_MOUSE) {

            // Check which button has been pressed...
            if(getmouse(&event) == OK) {

                // STOP button pressed
                if(check_button_pressed(stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "STP button pressed");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // RESET button pressed
                else if(check_button_pressed(rst_button, &event)) {
                    mvprintw(LINES - 1, 1, "RST button pressed");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }
        
        // To be commented in final version...
        switch (cmd)
        {
            case KEY_LEFT:
                x--;
                break;
            case KEY_RIGHT:
                x++;
                break;
            case KEY_UP:
                z--;
                break;
            case KEY_DOWN:
                z++;
                break;
            default:
                break;
        }
        
        // Update UI
        update_console_ui(&x, &z);
	}

    // Terminate
    endwin();
    return 0;
}

/* for solving the concatenation of the numbers in the pid array
for(i=0; i<= sizeof(pid_proces_rcv)) {
    pid_proces = pid_proces + pid_proces_rcv[i] * 10^i;
}*/