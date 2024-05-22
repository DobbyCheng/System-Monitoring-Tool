#ifndef STATS_FUNCTIONS_H
#define STATS_FUNCTIONS_H
#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utmp.h>

// function prototypes
void monitor_memory_usage(int samples, int tdelay, bool generateSquentially,
                          bool generateGraphics, int writePipe,
                          float *last_virtual, int i);
void monitor_user_usage(int samples, int tdelay, bool generateSquentially,
                        int writePipe);
void monitor_cpu_usage(int samples, int tdelay, bool generateSquentially,
                       bool generateGraphics, int writePipe, long *last_idle,
                       long *last_sum);
void terminate_child_processes();
void print_running_parameters(int samples, int tdelay, bool generateSquentially,
                              int i);
void print_system_information();
void get_CPU_info(long *idle, long *sum);

#endif