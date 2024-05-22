#include "stats_functions.h"

// function to print the running parameters
void print_running_parameters(int samples, int tdelay, bool generateSquentially,
                              int i) {
  if (generateSquentially) {
    printf(">>> iteration %d\n", i);  // print the iteration number
  } else {
    printf("\033[H\033[2J");  // clear the terminal screen
    printf("Nbr of samples: %d -- every %d secs\n", samples,
           tdelay);  // print number of samples and samples of frequency
  }

  struct rusage r_struct;
  getrusage(RUSAGE_SELF, &r_struct);
  printf(" Memory usage: %ld kilobytes\n",
         r_struct.ru_maxrss);  // print memory self-utilization
}

// function to print system information
void print_system_information() {
  // get name and information about current kernel
  struct utsname uts_struct;
  if (uname(&uts_struct) == -1) {
    fprintf(stderr, "Error calling uname()!\n");
    exit(1);
  }

  printf("---------------------------------------\n");
  printf("### System Information ### \n");
  printf(" System Name = %s\n", uts_struct.sysname);    // print system name
  printf(" Machine Name = %s\n", uts_struct.nodename);  // print machine name
  printf(" Version = %s\n", uts_struct.version);        // print OS version
  printf(" Release = %s\n", uts_struct.release);        // print OS release
  printf(" Architecture = %s\n",
         uts_struct.machine);  // print machine architecture

  // get system information
  struct sysinfo sys_struct;
  if (sysinfo(&sys_struct) == -1) {
    fprintf(stderr, "Error calling sysinfo()!\n");
    exit(1);
  }

  int seconds =
      sys_struct
          .uptime;  // get the system running time since last reboot in seconds
  int hours = seconds / 3600;  // calculate the number of hours
  seconds %= 3600;
  int minutes = seconds % 60;  // calculate the number of minutes
  seconds %= 60;
  if (hours >= 24 && hours < 48) {  // if the number of days is 1
    printf(
        " System running since last reboot: 1 day %02d:%02d:%02d "
        "(%02d:%02d:%02d)\n",
        hours % 24, minutes, seconds, hours, minutes, seconds);
  } else {  // if the number of days is greater than 1
    printf(
        " System running since last reboot: %d days %02d:%02d:%02d "
        "(%02d:%02d:%02d)\n",
        hours / 24, hours % 24, minutes, seconds, hours, minutes, seconds);
  }
  printf("---------------------------------------\n");
}

// function to print user usage to writePipe
void print_user_usage(int writePipe) {
  char outputStr[1025];
  snprintf(outputStr, sizeof(outputStr),
           "---------------------------------------\n");
  write(writePipe, outputStr, strlen(outputStr));
  snprintf(outputStr, sizeof(outputStr), "### Sessions/users ### \n");
  write(writePipe, outputStr, strlen(outputStr));

  setutent();  // rewind the file pointer to the beginning of the utmp file
  struct utmp *ut_pt = getutent();  // read a line from the utent file

  outputStr[0] = '\0';
  while (ut_pt != NULL) {
    if (ut_pt->ut_type ==
        USER_PROCESS) {  // check whether the user is connected
      snprintf(outputStr, sizeof(outputStr), " %s       %s (%s)\n",
               ut_pt->ut_user, ut_pt->ut_line,
               ut_pt->ut_host);  // print user information
      write(writePipe, outputStr, strlen(outputStr));
    }
    ut_pt = getutent();  // get the pointer to a user
  }

  endutent();  // close the utmp file

  snprintf(outputStr, sizeof(outputStr),
           "---------------------------------------\n");
  write(writePipe, outputStr, strlen(outputStr));
}

// function to get the CPU information
void get_CPU_info(long *idle, long *sum) {
  FILE *fp = fopen("/proc/stat", "r");  // open /proc/stat to read information
  if (fp == NULL) {  // if an error occurs when opening /proc/stat
    fprintf(stderr, "Error opening file\n");  // output error message
    exit(1);
  }

  char stat_buffer[256];
  fgets(stat_buffer, sizeof(stat_buffer),
        fp);  // store the content of the first line in stat_buffer
  fclose(fp);
  char *token = strtok(stat_buffer, " ");  // skip the name of the CPU
  *idle = 0;
  *sum = 0;

  for (int i = 0; token != NULL; ++i) {
    if (i ==
        7) {  // only add user, nice, sys, idle, IOwait, irq, softirq to sum
      break;
    }
    token = strtok(NULL, " ");  // get a parameter
    if (token == NULL) {        // if all the parameters have been gone over
      break;
    }
    *sum += atoi(token);  // add the number to the total cpu
    if (i == 3) {
      *idle += atoi(token);  // add idle in /proc/stat to idle_total
    }
  }
}

// funcation to calculate the percentage of CPU usage
float calc_CPU_usage_percent(long *last_idle, long *last_sum) {
  long idle, sum;
  get_CPU_info(&idle, &sum);  // get the CPU information for calculation

  float idle_delta =
      (float)(idle - *last_idle);  // the amount idle changes since last sample
  float sum_delta =
      (float)(sum -
              *last_sum);  // the amount total cpu changes since last sample
  float cpu_percent = ((sum_delta - idle_delta) / sum_delta) *
                      100;  // calculate the percentage for cpu usage

  *last_idle = idle;  // store idle as the last idle
  *last_sum = sum;    // store the total cpu as the last total cpu
  return cpu_percent;
}

// function to print CPU usage without graphics to writePipe and return the
// percentage of CPU usage
float print_CPU_usage(long *last_idle, long *last_sum, int writePipe) {
  char outputStr[1025];
  snprintf(outputStr, sizeof(outputStr), "Number of cores: %d \n",
           get_nprocs());  // print the number of cores
  write(writePipe, outputStr, strlen(outputStr));
  float cpu_percent = calc_CPU_usage_percent(
      last_idle, last_sum);  // store the percentage of CPU usage
  snprintf(outputStr, sizeof(outputStr), " total cpu use = %.2f%%\n",
           cpu_percent);  // print total CPU usage
  write(writePipe, outputStr, strlen(outputStr));
  return cpu_percent;
}

// function to generate the current line of graphics of CPU usage to writePipe
void generate_CPU_graphics(float current_cpu, int writePipe) {
  char outputStr[1025];
  strcpy(outputStr, "         ");
  int times = current_cpu;
  for (int j = 1; j <= times + 3;
       ++j) {  // add the desired number of '|' to the string
    strcat(outputStr, "|");
  }
  char str[32];
  sprintf(str, " %.2f", current_cpu);
  strcat(outputStr, str);  // add the cpu usage to the string
  write(writePipe, outputStr, strlen(outputStr));
}

// function to calculate the memory usage and return the virtual RAM memory used
float calc_memory_usage(char str[1025]) {
  // get system information and store it in sys_struct
  struct sysinfo sys_struct;
  if (sysinfo(&sys_struct) == -1) {
    fprintf(stderr, "Error calling sysinfo()!\n");
    exit(1);
  }
  float phys_used =
      (float)(sys_struct.totalram - sys_struct.freeram) / 1000 / 1000 /
      1000;  // calculate the physical RAM memory used (converted to GB)
  float phys_total =
      (float)sys_struct.totalram / 1000 / 1000 /
      1000;  // calculate the physical RAM memory in total (converted to GB)
  float virtual_used =
      phys_used +
      (float)(sys_struct.totalswap - sys_struct.freeswap) / 1000 / 1000 /
          1000;  // calculate the virtual RAM memory used (converted to GB)
  float virtual_total =
      (float)(sys_struct.totalram + sys_struct.totalswap) / 1000 / 1000 /
      1000;  // calculate the virtual RAM memory in total (converted to GB)

  // update the string with the memory information
  sprintf(str, "%.2f GB / %.2f GB -- %.2f GB / %.2f GB", phys_used, phys_total,
          virtual_used, virtual_total);
  return virtual_used;
}

// function to add memory usage graphics to the end of the memory usage report
void add_memory_graphics(float *last_virtual, float current_virtual, int i,
                         char str[1025]) {
  strcat(str, "   |");  // generate the delimiter

  float delta =
      (*last_virtual == -1.00)
          ? 0
          : (current_virtual -
             *last_virtual);  // calculate the change in virtual memory usage
  if (delta == 0.00) {        // generate the symbol for zero+
    strcat(str, "o ");
  } else {  // generate the symbols for changes that do not round to 0.00
    int times = delta * 100;
    for (int i = 1; i <= times; ++i) {
      strcat(str,
             (delta < 0) ? ":" : "#");  // ':' for negative and '#' for positve
    }
    strcat(str,
           (delta < 0) ? "@ " : "* ");  // '@' for negative and '*' for postive
  }

  char tmpStr[128];
  sprintf(tmpStr, "%.2f (%.2f)", delta, current_virtual);
  strcat(str, tmpStr);  // generate the changes in memory usage and the virtual
                        // memory usage

  *last_virtual =
      current_virtual;  // store the current virtual memory usage for later use
}

// function to monitor memory usage
void monitor_memory_usage(int samples, int tdelay, bool generateSquentially,
                          bool generateGraphics, int writePipe,
                          float *last_virtual, int i) {
  char str[1025];
  float current_virtual = calc_memory_usage(
      str);  // calculate memory usage and store virtual memory usage
  if (generateGraphics) {
    add_memory_graphics(last_virtual, current_virtual, i,
                        str);  // generate graphics for memory usage
  }
  write(writePipe, str, strlen(str));
  // close write pipe for child process
  if (close(writePipe) == -1) {
    fprintf(stderr, "Error closing the write end of the pipe!\n");
    exit(1);
  }
}

// function to monitor user usage
void monitor_user_usage(int samples, int tdelay, bool generateSquentially,
                        int writePipe) {
  print_user_usage(writePipe);  // print user usage to writePipe
                                // close write pipe for child process
  if (close(writePipe) == -1) {
    fprintf(stderr, "Error closing the write end of the pipe!\n");
    exit(1);
  }
}

// function to monitor cpu usage
void monitor_cpu_usage(int samples, int tdelay, bool generateSquentially,
                       bool generateGraphics, int writePipe, long *last_idle,
                       long *last_sum) {
  float current_cpu = 0.0;

  get_CPU_info(last_idle, last_sum);
  usleep(1000000);  // sleep 1s for cpu calculation time gap
  current_cpu = print_CPU_usage(last_idle, last_sum,
                                writePipe);  // print CPU usage tp writePipe
  if (generateGraphics) {
    generate_CPU_graphics(current_cpu,
                          writePipe);  // generate graphics for CPU usage
  }
  // close write pipe for child process
  if (close(writePipe) == -1) {
    fprintf(stderr, "Error closing the write end of the pipe!\n");
    exit(1);
  }
}