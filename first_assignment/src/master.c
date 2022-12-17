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
  int pipe; // declare the returned valeu of the funtion
  pipe = mkfifo(pathname,mode); // actually create the pipe
  if(pipe < 0) { // checking possible errors
    perror("error while creating the named pipe");
    return 1;
  }
}

int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) == 0);
    perror("Exec failed");
    return 1;
  }
}

void logger(char * log_pathname, char log_msg[]) {
  double c = (double) (clock() / CLOCKS_PER_SEC);
  char log_msg_arr[strlen(log_msg)+11];
  if ((sprintf(log_msg_arr, " %s,%.2E;", log_msg, c)) < 0){
    perror("error in logger sprintf");
  }
  int log_fd; // declare the log file descriptor
  if ((log_fd = open(log_pathname,  O_CREAT | O_APPEND | O_WRONLY, 0644)) < 0){ // open the log file to write on it
    perror(("error opening the log file %s", log_pathname)); // checking errors
  }
  if(write(log_fd, log_msg_arr, sizeof(log_msg_arr)) != sizeof(log_msg_arr)) { // writing the log message on the log file
      perror("error tring to write the log message in the log file"); // checking errors
  }
}

int main() {
  char * log_pn_master = "./log_files/master.txt"; // initialize the log file path name
  char * log_pn_command = "../bin/log_files/command.txt"; // initialize the log file path name
  char * log_pn_motor1 = "../bin/log_files/motor1.txt"; // initialize the log file path name
  char * log_pn_motor2 = "../bin/log_files/motor2.txt"; // initialize the log file path name
  char * log_pn_error = "../bin/log_files/error.txt"; // initialize the log file path name
  char * log_pn_inspect = "../bin/log_files/inspect.txt"; // initialize the log file path name

  logger(log_pn_master, "log legend: /n 0001= opened the pipes  0010= spawned the processes  0100= stop signal sended /n 0101= reset signal sended  0111= watchdogs has killed all the processes");
  // declering the mkpipe() arguments:
  // % from cmd to m1, m2:
  char *cmd_Vx_m1 = "../bin/named_pipes/Vx"; // pathname of the pipe from cmd to m1 through Vx
  mode_t cmd_Vx_m1_mode = 0666; // mode of the pipe from cmd to m1 through Vx
  
  char *cmd_Vz_m2 = "../bin/named_pipes/Vz"; // pathname of the pipe from cmd to m2 through Vz
  mode_t cmd_Vz_m2_mode = 0666; // mode of the pipe from cmd to m2 through Vz

  // % from m1,m2 to error:
  char *m1_x_err = "../bin/named_pipes/x"; // pathname of the pipe from m1 to error through x
  mode_t m1_x_err_mode = 0666; // mode of the pipe from m1 to error through x

  char *m2_z_err = "../bin/named_pipes/z"; // pathname of the pipe from m2 to error through z
  mode_t m2_z_err_mode = 0666; // mode of the pipe from m2 to error through z

  // % form error to inspect:
  char *err_x_c_ins = "../bin/named_pipes/x_c"; // pathname of the pipe from error to inspect through x_c
  mode_t err_x_c_ins_mode = 0666; // mode of the pipe from error to inspect through x_c

  char *err_z_c_ins = "../bin/named_pipes/z_c"; // pathname of the pipe from error to inspect through z_c
  mode_t err_z_c_ins_mode = 0666; // mode of the pipe from error to inspect through z_c

  // % from inspect to master for the signals:
  char *ins_s_mass = "../bin/named_pipes/s"; // pathname of the pipe from inspect to master through s
  mode_t ins_s_mass_mode = 0666; // mode of the pipe from inspect to master through s

  int fd_s; // declare the file descriptor for the pipe s
  int s_rcv[1]; // declare the array where i will store the signal id
  
  // opening the named pipes:
  mkpipe(cmd_Vx_m1, cmd_Vx_m1_mode); // creating the named pipe for communicate the speed along x between the cmd and the motor1 
  mkpipe(cmd_Vz_m2, cmd_Vz_m2_mode); // creating the named pipe for communicate the speed along z between the cmd and the motor2

  mkpipe(m1_x_err, m1_x_err_mode); // creating the named pipe for communicate the position along x between the m1 and the error
  mkpipe(m2_z_err, m2_z_err_mode); // creating the named pipe for communicate the position along z between the m2 and the error

  mkpipe(err_x_c_ins, err_x_c_ins_mode); // creating the named pipe for communicate the corrected position along x_c between the error and the inspect
  mkpipe(err_z_c_ins, err_z_c_ins_mode); // creating the named pipe for communicate the corrected position along z_c between the error and the inspect

  mkpipe(ins_s_mass, ins_s_mass_mode); // creating the named pipe for communicate the signal id from the inspect to the master

  logger(log_pn_master, "0001"); // write a log message
 
  // declaring the spawn() arguments:
  char * arg_list_command[] = { "../bin/konsole", "-e", "../bin/command", NULL };
  char * arg_list_m1[] = { "../bin/motor1", "-e", "../bin/motor1", NULL };
  char * arg_list_m2[] = { "../bin/motor2", "-e", "../bin/motor2", NULL };
  char * arg_list_err[] = { "../bin/error", "-e", "../bin/error", NULL };
  char * arg_list_insp[] = { "../bin/konsole", "-e", "../bin/inspection", NULL };

  pid_t pid_cmd = spawn("../bin/konsole", arg_list_command);
  pid_t pid_m1 = spawn("../bin/motor1", arg_list_m1);
  pid_t pid_m2 = spawn("../bin/motor2", arg_list_m2);
  pid_t pid_err = spawn("../bin/error", arg_list_err);
  pid_t pid_insp = spawn("../bin/konsole", arg_list_insp);

  logger(log_pn_master, "0010"); // write a log message

  // menaging the inspect signals:
  // open the pipes for the signals "stop" and "reset":
  fd_s = open(ins_s_mass, O_RDONLY); // open the pipe s to read on it
    if( ins_s_mass < 0){
        perror("error opening the pipe s from master"); // checking errors
    }
  
  logger(log_pn_master, "0011"); // write a log message

  while(1) {
    // read from the pipe and send the sgnals:
    if(read(fd_s, s_rcv, sizeof(s_rcv)) < 0) { // checking errors
      perror("error reading the pipe s from master"); 
    }
    else if (read(fd_s, s_rcv, sizeof(s_rcv)) > 0) { // otherwise read the signal id
      if (s_rcv[0] = 0) { // the inspect is asking to do the stop operation
        if (kill((pid_cmd, pid_m1, pid_m2), SIGUSR1) < 0) {
          perror("error while sending the signal to the cmd, m1, m2 from master");
        }

        logger(log_pn_master, "0100"); // write a log message

      }
      else if (s_rcv[0] = 1) { // the inspect is asking to do the reset operation
        if (kill((pid_cmd, pid_m1, pid_m2), SIGUSR2) < 0) {
          perror("error while sending the signal to the cmd, m1, m2 from master");
        }

        logger(log_pn_master, "0101"); // write a log message

      }
    }

    // Watchdogs:
    struct stat command_info; // declare the struct where i will store the command log file information
    struct stat motor1_info; // declare the struct where i will store the motor1 log file information
    struct stat motor2_info; // declare the struct where i will store the motor2 log file information
    struct stat error_info; // declare the struct where i will store the error log file information
    struct stat inspect_info; // declare the struct where i will store the inspect log file information
    time_t t; // declare the current time

    if(stat(log_pn_command, &command_info) < 0) { // save the log file information of the command in the struct
      perror("error while reading the command log file informations"); // check errors
    }
    if(stat(log_pn_motor1, &motor1_info) < 0) { // save the log file information of the motor1 in the struct
      perror("error while reading the motor1 log file informations"); // check errors
    }
    if(stat(log_pn_motor2, &motor2_info) < 0) { // save the log file information of the motor2 in the struct
      perror("error while reading the motor2 log file informations"); // check errors
    }
    if(stat(log_pn_error, &error_info) < 0) { // save the log file information of the error in the struct
      perror("error while reading the error log file informations"); // check errors
    }
    if(stat(log_pn_inspect, &inspect_info) < 0) { // save the log file information of the inspection in the struct
      perror("error while reading the inspect log file informations"); // check errors
    }
    
    t = time(NULL); // obtain the actual time
    if ((difftime(t, command_info.st_mtim.tv_sec) >= 60) | (difftime(t, motor1_info.st_mtim.tv_sec) >= 60) | (difftime(t, motor2_info.st_mtim.tv_sec) >= 60) | (difftime(t, error_info.st_mtim.tv_sec) >= 60) | (difftime(t, inspect_info.st_mtim.tv_sec) >= 60)) { // checking if one process is not active for 60 seconds
      if (kill((pid_cmd, pid_m1, pid_m2, pid_err, pid_insp), SIGKILL) < 0) { // kill all the processes
        perror("error while closing all the proces form the watchdogs"); // check errors
      }
      else {
        logger(log_pn_master, "0111"); // write a log message
        break; // exit the while loop
      }
    }
  }

  // closure
  int status[5]; // array where i will put the exit status of all the processes
  int *status_p = status; // pointer to the previous arrray
  waitpid(pid_cmd, status_p, 0); // wait for the proces to be closed and return the status
  remove(cmd_Vx_m1); // close the pipes
  remove(cmd_Vz_m2); // close the pipes
  waitpid(pid_m1, status_p+1, 0); // wait for the proces to be closed and return the status
  remove(m1_x_err); // close the pipes
  waitpid(pid_m2, status_p+2, 0); // wait for the proces to be closed and return the status
  remove(m2_z_err); // close the pipes
  waitpid(pid_err, status_p+3, 0); // wait for the proces to be closed and return the status
  remove(err_x_c_ins); // close the pipes
  remove(err_z_c_ins); // close the pipes
  waitpid(pid_insp, status_p+4, 0); // wait for the proces to be closed and return the status
  remove(ins_s_mass); // close the pipes
  
  printf ("the following process exited with the following errors: /n command console %d\n motor1 %d\n motor2 %d\n error %d\n inspection console %d\n", status[0], status[1], status[2], status[3], status[4]); // print why the processes where closed
  return 0;
}