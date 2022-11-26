#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

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

  // % from m1 to inspect:
  char *m1_x_ins = "./named_pipes/x"; // pathname of the pipe from m1 to inspect through x
  mode_t m1_x_ins_mode = 0666; // mode of the pipe from m1 to inspect through x

  // opening the named pipes:
  mkpipe(cmd_Vx_m1, cmd_Vx_m1_mode); // creating the named pipe for communicate the speed along x between the cmd and the motor1 
  mkpipe(cmd_Vz_m2, cmd_Vz_m2_mode); // creating the named pipe for communicate the speed along z between the cmd and the motor2

  mkpipe(m1_x_ins, m1_x_ins_mode); // creating the named pipe for communicate the speed along z between the cmd and the motor2

  // declaring the spawn() arguments:
  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", NULL };

  pid_t pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  // open the proces one by one %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  pid_t pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);

  int status;
  waitpid(pid_cmd, &status, 0);
  waitpid(pid_insp, &status, 0);
  
  printf ("Main program exiting with status %d\n", status);
  return 0;
}

// remembar to close the pipes when you've finished to use them