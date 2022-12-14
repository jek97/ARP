#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

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

int main() {

  // declering the mkpipe() arguments:
  // % from cmd to m1, m2:
  char *cmd_Vx_m1 = "./named_pipes/Vx"; // pathname of the pipe from cmd to m1 through Vx
  mode_t cmd_Vx_m1_mode = 0666; // mode of the pipe from cmd to m1 through Vx
  
  char *cmd_Vz_m2 = "./named_pipes/Vz"; // pathname of the pipe from cmd to m2 through Vz
  mode_t cmd_Vz_m2_mode = 0666; // mode of the pipe from cmd to m2 through Vz

  // % from m1,m2 to error:
  char *m1_x_err = "./named_pipes/x"; // pathname of the pipe from m1 to error through x
  mode_t m1_x_err_mode = 0666; // mode of the pipe from m1 to error through x

  char *m2_z_err = "./named_pipes/z"; // pathname of the pipe from m2 to error through z
  mode_t m2_z_err_mode = 0666; // mode of the pipe from m2 to error through z

  // % form error to inspect:
  char *err_x_c_ins = "./named_pipes/x_c"; // pathname of the pipe from error to inspect through x_c
  mode_t err_x_c_ins_mode = 0666; // mode of the pipe from error to inspect through x_c

  char *err_z_c_ins = "./named_pipes/z_c"; // pathname of the pipe from error to inspect through z_c
  mode_t err_z_c_ins_mode = 0666; // mode of the pipe from error to inspect through z_c

  // % from master to inspect, to communicate the pid of the processes command, m1, m2:
  // % % pipe to communicate the pid dimension:
  char *mas_pid_cmd_dim_ins = "./named_pipes/pid_cmd_dim"; // pathname of the pipe from master to inspect through pid_cmd_dim
  mode_t mas_pid_cmd_dim_ins_mode = 0666; // mode of the pipe from master to inspect through pid_cmd_dim

  char *mas_pid_m1_dim_ins = "./named_pipes/pid_m1_dim"; // pathname of the pipe from master to inspect through pid_m1_dim
  mode_t mas_pid_m1_dim_ins_mode = 0666; // mode of the pipe from master to inspect through pid_m1_dim

  char *mas_pid_m2_dim_ins = "./named_pipes/pid_m2_dim"; // pathname of the pipe from master to inspect through pid_m2_dim
  mode_t mas_pid_m2_dim_ins_mode = 0666; // mode of the pipe from master to inspect through pid_m2_dim

  // % % pipe to actually communicate the pid:
  char *mas_pid_cmd_ins = "./named_pipes/pid_cmd"; // pathname of the pipe from master to inspect through pid_cmd
  mode_t mas_pid_cmd_ins_mode = 0666; // mode of the pipe from master to inspect through pid_cmd

  char *mas_pid_m1_ins = "./named_pipes/pid_m1"; // pathname of the pipe from master to inspect through pid_m1
  mode_t mas_pid_m1_ins_mode = 0666; // mode of the pipe from master to inspect through pid_cmd

  char *mas_pid_m2_ins = "./named_pipes/pid_m2"; // pathname of the pipe from master to inspect through pid_m2
  mode_t mas_pid_m2_ins_mode = 0666; // mode of the pipe from master to inspect through pid_cmd

  // declaring the fd of the needed pipes:
  int fd_pid_cmd;
  int fd_pid_m1;
  int fd_pid_m2;
  int fd_pid_cmd_dim;
  int fd_pid_m1_dim;
  int fd_pid_m2_dim;

  // declaring the array where i will store the dimension of the pid:
  int cmd_dim[1];
  int m1_dim[1];
  int m2_dim[1];

  // declaring the array where i will store the pid to send them:
  int cmd[];
  int m1[];
  int m2[];
  
  // creating the named pipes:
  mkpipe(cmd_Vx_m1, cmd_Vx_m1_mode); // creating the named pipe for communicate the speed along x between the cmd and the motor1 
  mkpipe(cmd_Vz_m2, cmd_Vz_m2_mode); // creating the named pipe for communicate the speed along z between the cmd and the motor2

  mkpipe(m1_x_err, m1_x_err_mode); // creating the named pipe for communicate the position along x between the m1 and the error
  mkpipe(m2_z_err, m2_z_err_mode); // creating the named pipe for communicate the position along z between the m2 and the error

  mkpipe(err_x_c_ins, err_x_c_ins_mode); // creating the named pipe for communicate the corrected position along x_c between the error and the inspect
  mkpipe(err_z_c_ins, err_z_c_ins_mode); // creating the named pipe for communicate the corrected position along z_c between the error and the inspect

  mkpipe(mas_pid_cmd_dim_ins, mas_pid_cmd_dim_ins_mode); // creating the named pipe for communicate the dimension of the pid of the cmd between the master and the inspect
  mkpipe(mas_pid_m1_dim_ins, mas_pid_m1_dim_ins_mode); // creating the named pipe for communicate the the dimension of the pid of the m1 between the master and the inspect
  mkpipe(mas_pid_m2_dim_ins, mas_pid_m2_dim_ins_mode); // creating the named pipe for communicate the the dimension of the pid of the m2 between the master and the inspect

  mkpipe(mas_pid_cmd_ins, mas_pid_cmd_ins_mode); // creating the named pipe for communicate the pid of the cmd between the master and the inspect
  mkpipe(mas_pid_m1_ins, mas_pid_m1_ins_mode); // creating the named pipe for communicate the pid of the m1 between the master and the inspect
  mkpipe(mas_pid_m2_ins, mas_pid_m2_ins_mode); // creating the named pipe for communicate the pid of the m2 between the master and the inspect
 
  // declaring the spawn() arguments:
  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  char * arg_list_m1[] = { "/usr/bin/motor1", "-e", "./bin/motor1", NULL };
  char * arg_list_m2[] = { "/usr/bin/motor2", "-e", "./bin/motor2", NULL };
  char * arg_list_err[] = { "/usr/bin/error", "-e", "./bin/error", NULL };
  char * arg_list_insp[] = { "/usr/bin/konsole", "-e", "./bin/inspection", NULL };

  pid_t pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  pid_t pid_m1 = spawn("/usr/bin/motor1", arg_list_m1);
  pid_t pid_m2 = spawn("/usr/bin/motor2", arg_list_m2);
  pid_t pid_err = spawn("/usr/bin/error", arg_list_err);
  pid_t pid_insp = spawn("/usr/bin/konsole", arg_list_insp);


  // open the pipes from master to inspect for the pids dimension:
  fd_pid_cmd_dim = open(mas_pid_cmd_dim_ins, O_WRONLY); // open the pipe mas_pid_cmd_dim_ins to write on it
    if(fd_pid_cmd_dim < 0){
        perror("error opening the pipe mas_pid_cmd_dim_ins from master"); // checking errors
    }

  fd_pid_m1_dim = open(mas_pid_m1_dim_ins, O_WRONLY); // open the pipe mas_pid_m1_dim_ins to write on it
    if(fd_pid_m1_dim < 0){
        perror("error opening the pipe mas_pid_m1_dim_ins from master"); // checking errors
    }

  fd_pid_m2_dim = open(mas_pid_m2_dim_ins, O_WRONLY); // open the pipe mas_pid_m2_dim_ins to write on it
    if(fd_pid_m2_dim < 0){
        perror("error opening the pipe mas_pid_m2_dim_ins from master"); // checking errors
    }
  
  // open the pipes from master to inspect for the pids:
  fd_pid_cmd = open(mas_pid_cmd_ins, O_WRONLY); // open the pipe mas_pid_cmd_ins to write on it
    if(fd_pid_cmd < 0){
        perror("error opening the pipe mas_pid_cmd_ins from master"); // checking errors
    }

  fd_pid_m1 = open(mas_pid_m1_ins, O_WRONLY); // open the pipe mas_pid_m1_ins to write on it
    if(fd_pid_m1 < 0){
        perror("error opening the pipe mas_pid_m1_ins from master"); // checking errors
    }
  
  fd_pid_m2 = open(mas_pid_m2_ins, O_WRONLY); // open the pipe mas_pid_m2_ins to write on it
    if(fd_pid_m2 < 0){
        perror("error opening the pipe mas_pid_m2_ins from master"); // checking errors
    }
  
  // save the pid dimension of the processes in the relative array to send them:
  cmd_dim[] = sizeof(pid_cmd);
  m1_dim[] = sizeof(pid_m1);
  m2_dim[] = sizeof(pid_m2);
  // save the pid of the processes in the relative array to send them:
  cmd[] = pid_cmd;
  m1[] = pid_m1;
  m2[] = pid_m2;
  
  // send the pid dimension to the inspect:
  if(write(fd_pid_cmd_dim, cmd_dim, sizeof(cmd_dim)) != sizeof(cmd_dim)) { // writing the pid dimension on the pipe
    perror("error tring to write on the pid_cmd_dim pipe from master"); // checking errors
  }

  if(write(fd_pid_m1_dim, m1_dim, sizeof(m1_dim)) != sizeof(m1_dim)) { // writing the pid dimension on the pipe
    perror("error tring to write on the pid_m1_dim pipe from master"); // checking errors
  }

  if(write(fd_pid_m2_dim, m2_dim, sizeof(m2_dim)) != sizeof(m2_dim)) { // writing the pid dimension on the pipe
    perror("error tring to write on the pid_m2_dim pipe from master"); // checking errors
  }
  
  // send the pid to the inspect:
  if(write(fd_pid_cmd, cmd, sizeof(cmd)) != sizeof(cmd)) { // writing the pid on the pipe
    perror("error tring to write on the pid_cmd pipe from master"); // checking errors
  }

  if(write(fd_pid_m1, m1, sizeof(m1)) != sizeof(m1)) { // writing the pid on the pipe
    perror("error tring to write on the pid_m1 pipe from master"); // checking errors
  }

  if(write(fd_pid_m2, m2, sizeof(m2)) != sizeof(m2)) { // writing the pid on the pipe
    perror("error tring to write on the pid_m2 pipe from master"); // checking errors
  }
  
  
  

  int status;
  waitpid(pid_cmd, &status, 0);
  waitpid(pid_m1, &status, 0);
  waitpid(pid_m2, &status, 0);
  waitpid(pid_err, &status, 0);
  waitpid(pid_insp, &status, 0);
  
  printf ("Main program exiting with status %d\n", status);
  return 0;
}

// remembar to close the pipes when you've finished to use them