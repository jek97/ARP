#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
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

  // % from inspect to master for the signals:
  char *ins_s_mass = "./named_pipes/s"; // pathname of the pipe from inspect to master through s
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

  // open the pipes for the signals "stop" and "reset":
  fd_s = open(ins_s_mass, O_RDONLY); // open the pipe s to read on it
    if( ins_s_mass < 0){
        perror("error opening the pipe s from master"); // checking errors
    }
  
  // read from the pipe:
  if(read(fd_s, s_rcv, sizeof(s_rcv)) < 0) { // checking errors
      perror("error reading the pipe s from master"); 
  }
  else if (read(fd_s, s_rcv, sizeof(s_rcv)) > 0) { // otherwise read the signal id
      if (s_rcv[0] = 0) { // the inspect is asking to do the stop operation
        // do something
      }
      else if (s_rcv[0] = 1) { // the inspect is asking to do the reset operation
        // do something
      }
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