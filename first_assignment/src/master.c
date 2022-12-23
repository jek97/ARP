#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <string.h>

int mkpipe(const char *pathname, mode_t mode) { // function to create a named pipe
  int pipe; // declare the returned valeu of the mkfifo funtion
  int rm_pipe; // declare the returned valeu of the removing function
  remove(pathname); // remove any previous pipe with the same name
  pipe = mkfifo(pathname,mode); // actually create the pipe
  if(pipe < 0) { // checking possible errors
    perror("error while creating the named pipe");
    return -1;
  }
  else {
    return 1;
  }
}

int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return -1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) == 0);
    perror("Exec failed");
    return -1;
  }
}

int logger(char * log_pathname, char log_msg[]) {
  int log_fd; // declare the log file descriptor
  char log_msg_arr[strlen(log_msg)+11]; // declare the message string
  double c = (double) (clock() / CLOCKS_PER_SEC); // evaluate the time from the program launch
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

int main() {
  //logger:
  char * log_pn_master = "./bin/log_files/master.txt"; // initialize the log file path name
  char * log_pn_command = "./bin/log_files/command.txt"; // initialize the log file path name
  char * log_pn_motor1 = "./bin/log_files/motor1.txt"; // initialize the log file path name
  char * log_pn_motor2 = "./bin/log_files/motor2.txt"; // initialize the log file path name
  char * log_pn_error = "./bin/log_files/error.txt"; // initialize the log file path name
  char * log_pn_inspect = "./bin/log_files/inspect.txt"; // initialize the log file path name

  remove(log_pn_master); // remove the previous log file
  logger(log_pn_master, "log legend: 0001= opened the pipes  0010= spawned the processes 0011= opened the signal pipe  0100= stop signal sended  0101= reset signal sended  0111= watchdogs has killed all the processes  1000= pipe s readed  1001= removed the previous Vx pipe  1010= removed the previous Vz pipe  1011= removed the previous x pipe  1100= removed the previous z pipe  1101= removed the previous x_c pipe 1110= removed the previous z_c pipe  1111= removed the previous s pipe.    the log number with an e in front means the relative operation failed");
  
  // declering the mkpipe() arguments:
  // % from cmd to m1, m2:
  int cmd_Vx_m1_r; // inizialize the returned valeu from mkpipe
  char *cmd_Vx_m1 = "./bin/named_pipes/Vx"; // pathname of the pipe from cmd to m1 through Vx
  mode_t cmd_Vx_m1_mode = 0777; // mode of the pipe from cmd to m1 through Vx
  
  int cmd_Vz_m2_r; // inizialize the returned valeu from mkpipe
  char *cmd_Vz_m2 = "./bin/named_pipes/Vz"; // pathname of the pipe from cmd to m2 through Vz
  mode_t cmd_Vz_m2_mode = 0777; // mode of the pipe from cmd to m2 through Vz

  // % from m1,m2 to error:
  int m1_x_err_r; // inizialize the returned valeu from mkpipe
  char *m1_x_err = "./bin/named_pipes/x"; // pathname of the pipe from m1 to error through x
  mode_t m1_x_err_mode = 0777; // mode of the pipe from m1 to error through x

  int m2_z_err_r; // inizialize the returned valeu from mkpipe
  char *m2_z_err = "./bin/named_pipes/z"; // pathname of the pipe from m2 to error through z
  mode_t m2_z_err_mode = 0777; // mode of the pipe from m2 to error through z

  // % form error to inspect:
  int err_x_c_ins_r; // inizialize the returned valeu from mkpipe
  char *err_x_c_ins = "./bin/named_pipes/x_c"; // pathname of the pipe from error to inspect through x_c
  mode_t err_x_c_ins_mode = 0777; // mode of the pipe from error to inspect through x_c

  int err_z_c_ins_r; // inizialize the returned valeu from mkpipe
  char *err_z_c_ins = "./bin/named_pipes/z_c"; // pathname of the pipe from error to inspect through z_c
  mode_t err_z_c_ins_mode = 0777; // mode of the pipe from error to inspect through z_c

  // % from inspect to master for the signals:
  int ins_s_mass_r; // inizialize the returned valeu from mkpipe
  char *ins_s_mass = "./bin/named_pipes/s"; // pathname of the pipe from inspect to master through s
  mode_t ins_s_mass_mode = 0777; // mode of the pipe from inspect to master through s

  int fd_s; // declare the file descriptor for the pipe s
  int s_rcv[1]; // declare the array where i will store the signal id
  int * s_rcv_p = &s_rcv[0]; // initialize the pointer to the s_rcv array

  int k_wd_1;
  int k_wd_2;
  int k_wd_3;
  int k_wd_4;
  int k_wd_5;

  // declaring the spawn() arguments:
  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  char * arg_list_m1[] = { "./bin/motor1", "-e", "./bin/motor1", NULL };
  char * arg_list_m2[] = { "./bin/motor2", "-e", "./bin/motor2", NULL };
  char * arg_list_err[] = { "./bin/error", "-e", "./bin/error", NULL };
  char * arg_list_insp[] = { "/usr/bin/konsole", "-e", "./bin/inspection", NULL };

  // declaring some internal variables:
  int r_fd_s; // declare the returned valeu of the read function on the signal pipe
  int k_stop_1; // declaring the returned valeu of the kill function that stops the motors
  int k_stop_2; // declaring the returned valeu of the kill function that stops the motors
  int k_rst_1; // declaring the returned valeu of the kill function that reset the simulation
  int k_rst_2; // declaring the returned valeu of the kill function that reset the simulation
  
  // opening the named pipes:
  cmd_Vx_m1_r = mkpipe(cmd_Vx_m1, cmd_Vx_m1_mode); // creating the named pipe for communicate the speed along x between the cmd and the motor1 
  cmd_Vz_m2_r = mkpipe(cmd_Vz_m2, cmd_Vz_m2_mode); // creating the named pipe for communicate the speed along z between the cmd and the motor2

  m1_x_err_r = mkpipe(m1_x_err, m1_x_err_mode); // creating the named pipe for communicate the position along x between the m1 and the error
  m2_z_err_r = mkpipe(m2_z_err, m2_z_err_mode); // creating the named pipe for communicate the position along z between the m2 and the error

  err_x_c_ins_r = mkpipe(err_x_c_ins, err_x_c_ins_mode); // creating the named pipe for communicate the corrected position along x_c between the error and the inspect
  err_z_c_ins_r = mkpipe(err_z_c_ins, err_z_c_ins_mode); // creating the named pipe for communicate the corrected position along z_c between the error and the inspect

  ins_s_mass_r = mkpipe(ins_s_mass, ins_s_mass_mode); // creating the named pipe for communicate the signal id from the inspect to the master
  
  if (cmd_Vx_m1_r > 0 && cmd_Vz_m2_r > 0 && m1_x_err_r > 0 && m2_z_err_r > 0 && err_x_c_ins_r > 0 && err_z_c_ins_r > 0 && ins_s_mass_r > 0) {
    logger(log_pn_master, "0001"); // write a log message
  }
  else {
    logger(log_pn_master, "e0001"); // write a error log message
  }
  
  // spawn the processes:
  pid_t pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  pid_t pid_m1 = spawn("./bin/motor1", arg_list_m1);
  pid_t pid_m2 = spawn("./bin/motor2", arg_list_m2);
  pid_t pid_err = spawn("./bin/error", arg_list_err);
  pid_t pid_insp = spawn("/usr/bin/konsole", arg_list_insp);

  if(pid_cmd > 0 && pid_m1 > 0 && pid_m2 > 0 && pid_err > 0 && pid_insp > 0) {
    logger(log_pn_master, "0010"); // write a log message
  }
  else {
    logger(log_pn_master, "e0010"); // write a error log message
  }

  // opening the signal pipe
  fd_s = open(ins_s_mass, O_RDONLY | O_NONBLOCK); // open the pipe s to read on it
  if( fd_s < 0){
      perror("error opening the pipe s from master"); // checking errors
      logger(log_pn_master, "e0011"); // write a error log message
  }
  else {
    logger(log_pn_master, "0011"); // write a log message
  }

  while(1) {
    sleep(1);
    // menaging the inspect signals:
    // read from the pipe and send the sgnals:
    r_fd_s = read(fd_s, s_rcv_p, 1);
    if(r_fd_s < 0) { 
      perror("error reading the pipe s from master"); // checking errors
      logger(log_pn_master, "e1000"); // write a error log message
    }
    else if (r_fd_s > 0){ // otherwise read the signal id
      logger(log_pn_master, "1000"); // write a error log message
      if (s_rcv[0] == 0) { // the inspect is asking to do the stop operation
        k_stop_1 = kill(pid_m1, SIGUSR1); // send the signals
        k_stop_2 = kill(pid_m2, SIGUSR1); // send the signals
        if (k_stop_1 == 0 && k_stop_2 == 0){
          logger(log_pn_master, "0100"); // write a log message
        }
        else {
          perror("error while sending the signal to the cmd, m1, m2 from master"); // checking errors
          logger(log_pn_master, "e0100"); // write a error log message
        }
      }
      else if (s_rcv[0] == 1) { // the inspect is asking to do the reset operation
        k_rst_1 = kill(pid_m1, SIGUSR2); // send the signals
        k_rst_2 = kill(pid_m2, SIGUSR2); // send the signals
        if (k_rst_1 == 0 && k_rst_2 == 0){
          logger(log_pn_master, "0101"); // write a log message
        }
        else {
          perror("error while sending the signal to the cmd, m1, m2 from master"); // checking errors
          logger(log_pn_master, "e0101"); // write a error log message
        }
      }
    }
    memset(s_rcv_p, 0, sizeof(s_rcv)); // clear the receiving messages array
    
    // Watchdogs:
    struct stat command_info; // declare the struct where i will store the command log file information
    struct stat motor1_info; // declare the struct where i will store the motor1 log file information
    struct stat motor2_info; // declare the struct where i will store the motor2 log file information
    struct stat error_info; // declare the struct where i will store the error log file information
    struct stat inspect_info; // declare the struct where i will store the inspect log file information
    time_t t; // declare the current time

    if(stat(log_pn_command, &command_info) < 0) { // save the log file information of the command in the struct
      perror("error while reading the command log file informations"); // checking errors
    }
    if(stat(log_pn_motor1, &motor1_info) < 0) { // save the log file information of the motor1 in the struct
      perror("error while reading the motor1 log file informations"); // checking errors
    }
    if(stat(log_pn_motor2, &motor2_info) < 0) { // save the log file information of the motor2 in the struct
      perror("error while reading the motor2 log file informations"); // checking errors
    }
    if(stat(log_pn_error, &error_info) < 0) { // save the log file information of the error in the struct
      perror("error while reading the error log file informations"); // checking errors
    }
    if(stat(log_pn_inspect, &inspect_info) < 0) { // save the log file information of the inspection in the struct
      perror("error while reading the inspect log file informations"); // checking errors
    }
    
    t = time(NULL); // obtain the actual time
    if ((difftime(t, command_info.st_mtim.tv_sec) >= 60) | (difftime(t, motor1_info.st_mtim.tv_sec) >= 60) | (difftime(t, motor2_info.st_mtim.tv_sec) >= 60) | (difftime(t, error_info.st_mtim.tv_sec) >= 60) | (difftime(t, inspect_info.st_mtim.tv_sec) >= 60)) { // checking if one process is not active for 60 seconds
      
      k_wd_1 = kill(pid_cmd, SIGTERM); // kill the proces cmd
      k_wd_2 = kill(pid_m1, SIGTERM); // kill the proces m1
      k_wd_3 = kill(pid_m2, SIGTERM); // kill the proces m2
      k_wd_4 = kill(pid_err, SIGTERM); // kill the proces err
      k_wd_5 = kill(pid_insp, SIGTERM); // kill the proces insp

      if (k_wd_1 == 0 && k_wd_2 == 0 && k_wd_3 == 0 && k_wd_4 == 0 && k_wd_5 == 0) {
        logger(log_pn_master, "0111"); // write a log message
        break; // exit the while loop
      }
      else {
        perror("error while closing all the proces form the watchdogs"); // checking errors
        logger(log_pn_master, "e0111"); // write a error log message
      }
    }
  }
  
  // closure
  int status[5]; // array where i will put the exit status of all the processes
  int *status_p = status; // pointer to the previous arrray
  waitpid(pid_cmd, status_p, 0); // wait for the proces to be closed and return the status
  waitpid(pid_m1, status_p+1, 0); // wait for the proces to be closed and return the status
  waitpid(pid_m2, status_p+2, 0); // wait for the proces to be closed and return the status
  waitpid(pid_err, status_p+3, 0); // wait for the proces to be closed and return the status
  waitpid(pid_insp, status_p+4, 0); // wait for the proces to be closed and return the status

  printf ("the following process exited with the following errors: command console %d; motor1 %d; motor2 %d; error %d; inspection console %d;", status[0], status[1], status[2], status[3], status[4]); // print why the processes where closed
  return 0;
}