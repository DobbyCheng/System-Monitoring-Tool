
# System Monitoring Tool README

## Introduction

This document provides an overview and instructions for a system monitoring tool developed for Assignment 3. The tool monitors various system metrics like memory usage, CPU utilization, and user sessions concurrently, employing a modular and concurrent design approach.

## Problem Solving Approach

The program achieves concurrency by launching separate processes for monitoring memory, CPU, and user sessions. Communication between these child processes and the main process is accomplished through forks and pipes, ensuring synchronized and organized output consistent with Assignment 1 specifications. Signal handling for Ctrl-Z and Ctrl-C is implemented to ignore background execution attempts and prompt user confirmation before exit, respectively. When signal Ctrl-C is received and termination is confirmed, parent process will terminate all the child processes to avoid the appearance of orphan processes. Also, waitpid() is used to ensure consistency and to avoid the appearance of zombie processes.

## Overview of Functions

The System Monitoring Tool comprises several functions designed for modular and efficient monitoring of system resources. Here's an overview of each key function:

- `void monitor_memory_usage(int interval, int duration)`: Monitors and reports memory usage over specified intervals (in seconds) and total duration (in seconds). This function samples the system's memory usage at each interval, displaying the percentage of memory used. The output is formatted and sent through a pipe to the main process for display or logging. Parameters include the sampling interval and the total duration for monitoring.

- `void monitor_cpu_usage(int interval, int duration, bool graphical_output)`: Gathers and reports CPU usage over specified intervals and duration, with an option for graphical output. The function calculates CPU usage percentage and can render a graphical representation if `graphical_output` is set to true. Like memory monitoring, data is sent through a pipe for main process handling. Parameters allow customization of the monitoring interval, duration, and output format.

- `void monitor_user_usage()`: Tracks and reports user session data, providing insights into active sessions. This function lists currently logged-in users and their activity status, sending this information through a pipe to the main process for output. It's designed to be called periodically to track session changes over time.

Additional utility functions support these monitoring tasks by facilitating the cleanup of data structures, retrieval of system information, and safe termination of child processes.

## Compilation and Running Instructions

### Compiling the Program
Navigate to the source directory and execute:
```
make all
```

### Running the Program
Execute the compiled binary with optional flags (`--graphics` for graphical output):
```
./system [--graphics]
```

### Additional Commands
- Clean compiled files: `make clean`
- Display help: `make help`

### Signal Handling
- Ctrl-Z is ignored to prevent background execution.
- Ctrl-C handling prompts the user before exiting, ensuring intentional program termination.

For a detailed understanding of runtime arguments and their effects, refer to the source code documentation and argument parsing logic within `system.c`.

## Additional Notes

The tool's design strictly follows the assignment's specifications, including modular coding practices, error checking, and avoiding global variables. It is tailored for Linux environments, particularly those used in academic settings.

