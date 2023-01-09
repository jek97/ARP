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

int main() {
  // directory variables:
  const char * log_dir = "./bin/log_files"; // initialize the pathname of the log directory
  mode_t log_dir_mode = 0777; // initialize the log directory mode

  // logger variable:
  const char * log_pn_master = "./bin/log_files/master.txt"; // initialize the log file path name

  char * arg_list_A[] = { "/usr/bin/konsole", "-e", "./bin/processA", NULL };
  char * arg_list_B[] = { "/usr/bin/konsole", "-e", "./bin/processB", NULL };

  logger(log_pn_master, "log legend: 0001= opened/created the log files folder   0010= spawned the processes   0011= processes have been closed   the log number with an e in front means the relative operation failed"); // write a log message
  
  // create the directories for the log files:
  if(opendir(log_dir) == NULL) { // try to open the directory to check if it exists
    if (mkdir(log_dir, log_dir_mode) < 0) { // create the directory
      perror("error creating the log_file directory from master"); // checking errors
      logger(log_pn_master, "e0001"); // write a log message
    }
    else {
      logger(log_pn_master, "0001"); // write a log message
    }
  }

  remove(log_pn_master); // remove the old log file

  // spawn the processes:
  pid_t pid_procA = spawn("/usr/bin/konsole", arg_list_A);
  sleep(1); // wait for the first process to create the shared memory and so on
  pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);

  if (pid_procA < 0 && pid_procB < 0) {
    logger(log_pn_master, "e0010"); // write a log message
  }
  else {
    logger(log_pn_master, "0010"); // write a log message
  }
  int status;
  waitpid(pid_procA, &status, 0);
  waitpid(pid_procB, &status, 0);
  logger(log_pn_master, "0011"); // write a log message
  printf ("Main program exiting with status %d\n", status);
  return 0;
}

