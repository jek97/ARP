#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>


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

int main() {
  // directory variables:
  const char * log_dir = "./bin/log_files"; // initialize the pathname of the log directory
  mode_t log_dir_mode = 0777; // initialize the log directory mode
  const char * named_pipes_dir = "./bin/named_pipes"; // initialize the pathname of the log directory
  mode_t named_pipes_dir_mode = 0777; // initialize the log directory mode

  // logger variable:
  const char * log_pn_master = "./bin/log_files/master.txt"; // initialize the log file path name
  
  // spawning processes variables
  char * arg_list_A[] = { "/usr/bin/konsole", "-e", "./bin/processA", NULL }; // normal mode
  char * arg_list_As[] = { "/usr/bin/konsole", "-e", "./bin/processAs", NULL }; // server mode
  char * arg_list_Ac[] = { "/usr/bin/konsole", "-e", "./bin/processAc", NULL }; // client mode
  char * arg_list_B[] = { "/usr/bin/konsole", "-e", "./bin/processB", NULL };

  // pipes variable:
  int s_mass_r; // inizialize the returned valeu from mkpipe
  const char *s_mass = "./bin/named_pipes/s"; // pathname of the pipe from inspect to master through s
  mode_t s_mass_mode = 0777; // mode of the pipe from inspect to master through s
  int fd_s; // declare the file descriptor for the pipe s
  int s_rcv[1]; // declare the array where i will store the signal id
  int * s_rcv_p = &s_rcv[0]; // initialize the pointer to the s_rcv array
  ssize_t r_fd_s; // declare the returned valeu of the read function on the signal pipe
  int k; // declaring the returned valeu of the kill function that close processB
  
  // processA type chose variables:
  int t; // declare the variable that i will use to switch between processA normal/server/client

  // closure
  int status[2]; // array where i will put the exit status of all the processes
  int *status_p = status; // pointer to the previous arrray
  
  // create the directories for the log files and the named pipes:
  if(opendir(log_dir) == NULL) { // try to open the directory to check if it exists
    if (mkdir(log_dir, log_dir_mode) < 0) { // create the directory
      perror("error creating the log_file directory from master"); // checking errors
      logger(log_pn_master, "e0001"); // write a log message
    }
    else {
      logger(log_pn_master, "0001"); // write a log message
    }
  }

  if(opendir(named_pipes_dir) == NULL) { // try to open the directory to check if it exists
    if (mkdir(named_pipes_dir, named_pipes_dir_mode) < 0) { // create the directory
      perror("error creating the named_pipes directory from master"); // checking errors
      logger(log_pn_master, "e0010"); // write a log message
    }
    else {
      logger(log_pn_master, "0010"); // write a log message
    }
  }

  remove(log_pn_master); // remove the old log file
  logger(log_pn_master, "log legend: 0001= opened/created the log files folder   0010= opened/created the named pipes folder   0011= spawned the processes   0100= created the pipe s   0101= opened the pipe s   0110= readed the pipe s and signal sended   0111= closed the pipe s   1000= unlink the pipe s   1001= wait for the returning status of the processes   1010= open processA in normal mode   1011= open processA in server mode   1100= open processA in client mode   the log number with an e in front means the relative operation failed"); // write a log message
  
  // ask for which type of processA open:
  printf("Please press 0 to open the processA in normal mode, 1 to open it in server mode or 2 to open it in client mode"); // ask for witch processA open
  scanf("%i", &t); // read the answer
  
  // spawn the processes based on the processA type:
  switch (t) {
    case 0: // processA normal mode
    printf("normal mode");
    pid_t pid_procA = spawn("/usr/bin/konsole", arg_list_A); // open the related process
    logger(log_pn_master, "1010"); // write a log message
    sleep(0.8); // wait for the first process to create the shared memory and so on
    pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);

    case 1: // processA server mode
    printf("server mode");
    pid_t pid_procA = spawn("/usr/bin/konsole", arg_list_As); // open the related process
    logger(log_pn_master, "1011"); // write a log message

    case 2: // processA client mode
    printf("client mode");
    pid_t pid_procA = spawn("/usr/bin/konsole", arg_list_Ac); // open the related process
    logger(log_pn_master, "1100"); // write a log message
    sleep(0.8); // wait for the first process to create the shared memory and so on
    pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);
  }
  
  if (pid_procA < 0 && pid_procB < 0) {
    logger(log_pn_master, "e0011"); // write a log message
  }
  else {
    logger(log_pn_master, "0011"); // write a log message
  }

  // create the pipe:
  s_mass_r = mkpipe(s_mass, s_mass_mode); // creating the named pipe for communicate the signal id from the processA to the master
  if (s_mass_r < 0) {
    perror("error creating the pipe s from master"); // checking errors
    logger(log_pn_master, "e0100"); // write a log message
  }
  else {
    logger(log_pn_master, "0100"); // write a log message
  }

  // opening the signal pipe
  fd_s = open(s_mass, O_RDONLY | O_NONBLOCK); // open the pipe s to read on it
  if( fd_s < 0){
      perror("error opening the pipe s from master"); // checking errors
      logger(log_pn_master, "e0101"); // write a error log message
  }
  else {
    logger(log_pn_master, "0101"); // write a log message
  }

  while(1) {
    r_fd_s = read(fd_s, s_rcv_p, 1);
    if(r_fd_s < 0 && errno != EAGAIN) { 
      perror("error reading the pipe s from master"); // checking errors
      logger(log_pn_master, "e0110"); // write a error log message
    }
    else if (r_fd_s > 0){ // otherwise read the signal id
      logger(log_pn_master, "0110"); // write log message
      if (s_rcv[0] == 1) { // the inspect is asking to close the processes
        k = kill(pid_procB, SIGUSR1); // send the signals
        break;
      }
    }
  }

  if (close(fd_s) < 0) {
    perror("error closing the pipe s from master"); // checking errors
    logger(log_pn_master, "e0111"); // write a error log message
  }
  else {
    logger(log_pn_master, "0111"); // write a error log message
  }
  
  if (unlink(s_mass) < 0) {
    perror("error unlinking the pipe s from master"); // checking errors
    logger(log_pn_master, "e1000"); // write a error log message
  }
  else {
    logger(log_pn_master, "1000"); // write a error log message
  }

  waitpid(pid_procA, status_p, 0); // wait for the proces to be closed and return the status
  waitpid(pid_procB, status_p+1, 0); // wait for the proces to be closed and return the status
  
  logger(log_pn_master, "1001"); // write a log message
  printf ("the following process exited with the following errors: processA %d; processB %d;", status[0], status[1]); // print why the processes where closed
  return 0;
}