#include "stats_functions.h"

// initialize or access the head of child pid linked list
pid_t *get_child_pid(bool op) {
  static pid_t childPid[3];
  if (op == true) {  // clean option
    childPid[0] = childPid[1] = childPid[2] = 0;
    return 0;
  }
  return childPid;
}

// function for child process to handle Ctrl+C: ignore the interruption signal
void child_handle_interruption(int sig) {}

// function for memory-monitoring child process to handle termination
void child_memory_handle_termination(int sig) {
  // fprintf(stdout, "memory child is killed\n");
  exit(0);
}
// function for cpu-monitoring child process to handle termination
void child_cpu_handle_termination(int sig) {
  // fprintf(stdout, "cpu child is killed\n");
  exit(0);
}
// function for user-monitoring child process to handle termination
void child_user_handle_termination(int sig) {
  // fprintf(stdout, "user child is killed\n");
  exit(0);
}

// double check with user about whether or not to terminate the program
bool termination_double_check() {
  char ch;
  printf("Do you want to quit the program? (Y/y for termination)\n");
  scanf("%c", &ch);
  return (ch == 'Y' || ch == 'y');  // Y/y for termination
}

// function to kill the remaining child processes
void terminate_child_processes() {
  pid_t *pidArray =
      get_child_pid(false);  // get the first address of the child-pid array
  for (int i = 0; i < 3; ++i) {
    pid_t childPid = pidArray[i];
    if (childPid != 0) {             // child pid is valid
      if (kill(childPid, 0) == 0) {  // child process is alive
        // printf("terminating %d\n", childPid);
        if (kill(childPid, SIGTERM) == -1) {  // error killing
          perror("Error killing child process!\n");
          exit(1);
        }
      } else if (errno != ESRCH) {  // error checking alive status
        perror("Error checking child process status!\n");
        exit(1);
      }
    }
  }
}

// function for parent process to handle Ctrl+C
void parent_handle_interruption(int sig) {
  bool terminationStatus =
      termination_double_check();  // double check termination
  if (terminationStatus) {         // user confirm to terminate the program
    terminate_child_processes();
    // fprintf(stdout, "parent is killed\n");
    exit(0);
  }
}

// function to update a child pid at position pos of the pid linked list
void update_pid(pid_t childPid, int pos) {
  pid_t *pidArray =
      get_child_pid(false);  // get the first address of the child-pid array
  pidArray[pos] = childPid;
}

// function to check whether the string is an integer
bool isInteger(const char *str) {
  if (!*str) return false;  // empty string

  char *ptr;
  strtol(str, &ptr, 10);  // attempt to convert str to a long
  return *ptr == '\0';    // check whether str can be fully converted to a long
                          // (can be considered as an integer)
}

// function to parse the command line arguments
void parse_command_line_arguments(int argc, char *argv[], int *samples,
                                  int *tdelay, bool *generateSystem,
                                  bool *generateUser, bool *generateGraphics,
                                  bool *generateSquentially, int *samplesStatus,
                                  int *tdelayStatus) {
  if (argc > 1) {  // at least one command line argument except the first one
    for (int i = 1; i < argc; ++i) {
      char *token = strtok(argv[i], "=");  // get the part of argv[i] before '='
      if (!strcmp(argv[i],
                  "--system")) {  // argv[i] equals "--system", meaning only
                                  // system usage should be generated
        *generateUser = false;
      } else if (!strcmp(argv[i],
                         "--user")) {  // argv[i] equals "--user", meaning only
                                       // user usage should be generated
        *generateSystem = false;
      } else if (!strcmp(argv[i], "--graphics") ||
                 !strcmp(argv[i],
                         "-g")) {  // argv[i] equals "--graphics", meaning
                                   // graphical output should be generated
        *generateGraphics = true;
      } else if (!strcmp(argv[i],
                         "--sequential")) {  // argv[i] equals "--sequential",
                                             // meaning the reports should be
                                             // generated sequentially
        *generateSquentially = true;
      } else if (!strcmp(
                     token,
                     "--samples")) {  // argv[i] equals "--samples=N", meaning N
                                      // samples should be collected
        *samples = atoi(strtok(NULL, ""));
        if (*samplesStatus == 0) {
          *samplesStatus = 2;
        } else if (*samplesStatus == 1) {
          *samplesStatus = 3;
        }
      } else if (!strcmp(token,
                         "--tdelay")) {  // argv[i] equals "--tdelay=T", meaning
                                         // sampling frequency is tdelay seconds
        *tdelay = atoi(strtok(NULL, ""));
        if (*tdelayStatus == 0) {
          *tdelayStatus = 2;
        } else if (*tdelayStatus == 1) {
          *tdelayStatus = 3;
        }
      } else if (isInteger(argv[i])) {  // argv[i] is an integer
        if (*samplesStatus == 0 ||
            *samplesStatus == 2) {  // sample is not specified by positional
          if (*samplesStatus == 0) {
            *samples = atoi(argv[i]);
            *samplesStatus = 1;
          } else {
            *samplesStatus = 3;
          }
        } else if (*tdelayStatus == 0 ||
                   *tdelayStatus ==
                       2) {  // delay is not specified by positional
          if (*tdelayStatus == 0) {
            *tdelay = atoi(argv[i]);
            *tdelayStatus = 1;
          } else {
            *tdelayStatus = 3;
          }
        } else {  // more than 2 positional arguments detected
          fprintf(stderr,
                  "There should be no more than 2 positional arguments!\n");
          exit(1);
        }
      } else {  // the command line arguments are invalid inputs
        fprintf(stderr, "Invalid command line arguments!\n");
        exit(1);
      }
    }
  }

  // cases for --system and --user both occur in the command line arguments
  if (!*generateSystem && !*generateUser) {
    *generateSystem = true;
    *generateUser = true;
  }
}

// function to print memory usage
void print_memory_usage(int samples, bool generateSquentially, int i,
                        char memoryStr[][1024]) {
  printf("---------------------------------------\n");
  // print the first line of memory report
  printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
  if (!generateSquentially) {  // if report does not need to be generated
                               // sequentially
    for (int j = 0; j <= i; ++j) {
      printf("%s\n", memoryStr[j]);
    }
    for (int j = i + 1; j < samples; ++j) {
      printf("\n");
    }
  } else {  // if report needs to be generated sequentially
    // print the report only on the current line
    for (int j = 0; j < samples; ++j) {
      if (j == i) {
        printf("%s\n", memoryStr[j]);
      } else {
        printf("\n");
      }
    }
  }
}
// function to print the graphics of CPU usage
void print_CPU_graphics(bool generateSquentially, int i, char cpuStr[][1024]) {
  if (!generateSquentially) {  // if report needs to be generated sequentially
    for (int j = 0; j <= i; ++j) {
      printf("%s\n", cpuStr[j]);
    }
  } else {  // if report does not need to be generated sequentially
    // only print the current string on one line
    for (int j = 0; j < i; ++j) {
      printf("\n");
    }
    printf("%s\n", cpuStr[i]);
  }
}

// function using fork to create processes for memory usage monitoring
void fork_memory(pid_t *pidForMemory, int pipeForMemory[2], int samples,
                 int tdelay, bool generateSquentially, bool generateGraphics,
                 float *last_virtual, int i) {
  *pidForMemory = fork();
  if (*pidForMemory == 0) {  // this is child process
    // for child process to handle Ctrl+C
    if (signal(SIGINT, child_handle_interruption) == SIG_ERR) {
      fprintf(stderr, "Error setting a signal handler!\n");
      exit(1);
    }
    // for child process to handle termination
    if (signal(SIGTERM, child_memory_handle_termination) == SIG_ERR) {
      fprintf(stderr, "Error setting a signal handler!\n");
      exit(1);
    }
    close(pipeForMemory[0]);  // close read end of child process
    if (i > 0) {
      int ms = tdelay * 1000000;
      usleep(ms);  // wait for certain seconds
    }
    monitor_memory_usage(samples, tdelay, generateSquentially, generateGraphics,
                         pipeForMemory[1], last_virtual, i);
    exit(0);
  } else if (*pidForMemory == -1) {  // error handling
    fprintf(stderr, "Error forking processes for memory usage monitoring!\n");
    terminate_child_processes();
    exit(1);
  } else {  // this is parent process
    update_pid(*pidForMemory, 0);
    // close write end of parent process
    if (close(pipeForMemory[1]) == -1) {
      fprintf(stderr, "Error closing the write end of the pipe!\n");
      exit(1);
    }
  }
}

// function using fork to create processes for user usage monitoring
void fork_user(pid_t *pidForUser, int pipeForUser[2], int samples, int tdelay,
               bool generateSquentially, bool generateGraphics, int i) {
  *pidForUser = fork();
  if (*pidForUser == 0) {  // this is child process
    // for child process to handle Ctrl+C
    if (signal(SIGINT, child_handle_interruption) == SIG_ERR) {
      fprintf(stderr, "Error setting a signal handler!\n");
      exit(1);
    }
    // for child process to handle termination
    if (signal(SIGTERM, child_memory_handle_termination) == SIG_ERR) {
      fprintf(stderr, "Error setting a signal handler!\n");
      exit(1);
    }
    close(pipeForUser[0]);  // close read end of child process
    if (i > 0) {
      int ms = tdelay * 1000000;
      usleep(ms);  // wait for certain seconds
    }
    monitor_user_usage(samples, tdelay, generateSquentially, pipeForUser[1]);
    exit(0);
  } else if (*pidForUser == -1) {  // error handling
    fprintf(stderr, "Error forking processes for user usage monitoring!\n");
    terminate_child_processes();
    exit(1);
  } else {  // this is parent process
    update_pid(*pidForUser, 1);
    // close write end of parent process
    if (close(pipeForUser[1]) == -1) {
      fprintf(stderr, "Error closing the write end of the pipe!\n");
      exit(1);
    }
  }
}

// function using fork to create processes for cpu usage monitoring
void fork_cpu(pid_t *pidForCPU, int pipeForCPU[2], int samples, int tdelay,
              bool generateSquentially, bool generateGraphics, long *last_idle,
              long *last_sum, int i) {
  *pidForCPU = fork();
  if (*pidForCPU == 0) {  // this is child process
    // for child process to handle Ctrl+C
    if (signal(SIGINT, child_handle_interruption) == SIG_ERR) {
      fprintf(stderr, "Error setting a signal handler!\n");
      exit(1);
    }
    // for child process to handle termination
    if (signal(SIGTERM, child_memory_handle_termination) == SIG_ERR) {
      fprintf(stderr, "Error setting a signal handler!\n");
      exit(1);
    }
    close(pipeForCPU[0]);  // close read end of child process
    if (i > 0) {
      int ms = (tdelay - 1) * 1000000;
      usleep(ms);  // wait for certain seconds
    }
    monitor_cpu_usage(samples, tdelay, generateSquentially, generateGraphics,
                      pipeForCPU[1], last_idle, last_sum);
    exit(0);
  } else if (*pidForCPU == -1) {  // error handling
    fprintf(stderr, "Error forking processes for user usage monitoring!\n");
    terminate_child_processes();
    exit(1);
  } else {  // this is parent process
    update_pid(*pidForCPU, 2);
    // close write end of parent process
    if (close(pipeForCPU[1]) == -1) {
      fprintf(stderr, "Error closing the write end of the pipe!\n");
      exit(1);
    }
  }
}

// function to print all the stats needed to be reported
void print_all_stats(int samples, int tdelay, bool generateSystem,
                     bool generateUser, bool generateSquentially,
                     bool generateGraphics) {
  char bufferStr[1024], outputStr[1024];
  char memoryStr[samples][1024], cpuStr[samples][1024];
  ssize_t bytesRead;
  int pipeForMemory[2], pipeForUser[2], pipeForCPU[2];  // pipes definition
  pid_t pidForMemory, pidForUser, pidForCPU;            // pid definition
  float last_virtual = -1.0;
  long last_idle = 0, last_sum = 0;

  for (int i = 0; i < samples; ++i) {
    if (!generateSystem) {  // system usage should not be generated
      // create pipes for user-monitoring
      if (pipe(pipeForUser) == -1) {
        fprintf(stderr, "Error creating pipes!\n");
        terminate_child_processes();
        exit(1);
      }
      // update the pid array
      update_pid(0, 0);
      fork_user(&pidForUser, pipeForUser, samples, tdelay, generateSquentially,
                generateGraphics, i);
      update_pid(0, 2);
      // Wait for child process to finish
      waitpid(pidForUser, NULL, 0);
      // print the running parameters
      print_running_parameters(samples, tdelay, generateSquentially, i);
      // print user usage
      while ((bytesRead = read(pipeForUser[0], bufferStr, sizeof(bufferStr))) >
             0) {
        printf("%.*s", (int)bytesRead, bufferStr);
      }
      if (bytesRead == -1) {  // error reading
        perror("Erroring reading from pipes!\n");
        terminate_child_processes();
        exit(1);
      }
      if (close(pipeForUser[0]) == -1) {  // error closing pipe end
        fprintf(stderr, "Error closing the read end of the pipe!\n");
        exit(1);
      }
    } else {  // system usage should be generated
      // create pipes
      if (pipe(pipeForMemory) == -1 || pipe(pipeForUser) == -1 ||
          pipe(pipeForCPU) == -1) {
        fprintf(stderr, "Error creating pipes!\n");
        terminate_child_processes();
        exit(1);
      }

      // use fork to create child processes
      fork_memory(&pidForMemory, pipeForMemory, samples, tdelay,
                  generateSquentially, generateGraphics, &last_virtual, i);
      if (generateUser) {
        fork_user(&pidForUser, pipeForUser, samples, tdelay,
                  generateSquentially, generateGraphics, i);
      } else {
        update_pid(0, 1);
      }
      fork_cpu(&pidForCPU, pipeForCPU, samples, tdelay, generateSquentially,
               generateGraphics, &last_idle, &last_sum, i);

      waitpid(pidForMemory, NULL, 0);  // Wait for child process to finish
      // print the running parameters
      print_running_parameters(samples, tdelay, generateSquentially, i);
      if ((bytesRead = read(pipeForMemory[0], bufferStr, sizeof(bufferStr))) ==
          -1) {  // error reading
        perror("Erroring reading from pipes!\n");
        terminate_child_processes();
        exit(1);
      }
      bufferStr[bytesRead] = '\0';
      // store the string into array
      strcpy(memoryStr[i], bufferStr);
      // print memory usage
      print_memory_usage(samples, generateSquentially, i, memoryStr);

      if (generateUser) {
        waitpid(pidForUser, NULL, 0);  // Wait for child process to finish
        // print user usage
        while ((bytesRead =
                    read(pipeForUser[0], bufferStr, sizeof(bufferStr))) > 0) {
          printf("%.*s", (int)bytesRead, bufferStr);
        }
        if (bytesRead == -1) {  // error reading
          perror("Erroring reading from pipes!\n");
          terminate_child_processes();
          exit(1);
        }
      }

      waitpid(pidForCPU, NULL, 0);  // Wait for child process to finish
      if ((bytesRead = read(pipeForCPU[0], bufferStr, sizeof(bufferStr))) ==
          -1) {  // error reading
        perror("Erroring reading from pipes!\n");
        terminate_child_processes();
        exit(1);
      }
      // get the last string
      int strPointer = bytesRead - 1;
      while (bufferStr[strPointer] != '\n') {
        --strPointer;
      }
      int length = bytesRead - (strPointer + 1);
      memset(outputStr, '\0', length + 1);
      strncpy(outputStr, &bufferStr[strPointer + 1], length);
      // store the string into array
      strcpy(cpuStr[i], outputStr);
      bufferStr[strPointer + 1] = '\0';

      // print cpu usage
      printf("%s", bufferStr);
      if (generateGraphics) {
        print_CPU_graphics(generateSquentially, i, cpuStr);
      }
      // close the read ends of parent process
      if (close(pipeForMemory[0]) == -1) {
        fprintf(stderr, "Error closing the read end of the pipe!\n");
        exit(1);
      }
      if (close(pipeForUser[0]) == -1) {
        fprintf(stderr, "Error closing the read end of the pipe!\n");
        exit(1);
      }
      if (close(pipeForCPU[0]) == -1) {
        fprintf(stderr, "Error closing the read end of the pipe!\n");
        exit(1);
      }
    }
    // printf("end of %d's iteration!\n",i);
  }

  print_system_information();  //  print system information
}

int main(int argc, char *argv[]) {
  int samples = 10, tdelay = 1;  // number of samples and sample frequency
  bool generateSystem = true;    // whether or not to generate the system usage
  bool generateUser = true;      // whether or not to generate the users usage
  bool generateGraphics =
      false;  // whether or not to generate the graphical output
  bool generateSquentially = false;  // whether or not to generate sequentially
  int samplesStatus =
      0;  // status = 0: not specified; 1: specified by positional
          // 2: specified by flag; 3: both flag & positional
  int tdelayStatus =
      0;  // status = 0: not specified; 1: specified by positional
          // 2: specified by flag; 3: both flag & positional

  // ignore signal Ctrl+Z when it is received
  if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
    fprintf(stderr, "Error setting a signal handler!\n");
    exit(1);
  }
  // for parent process to handle Ctrl+C
  if (signal(SIGINT, parent_handle_interruption) == SIG_ERR) {
    fprintf(stderr, "Error setting a signal handler!\n");
    terminate_child_processes();
    exit(1);
  }

  // deal with command line arguments
  parse_command_line_arguments(
      argc, argv, &samples, &tdelay, &generateSystem, &generateUser,
      &generateGraphics, &generateSquentially, &samplesStatus, &tdelayStatus);

  // print all the stats
  print_all_stats(samples, tdelay, generateSystem, generateUser,
                  generateSquentially, generateGraphics);
  return 0;
}