#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <shared.h>

#define MAX_PROCESSES 100
#define LOG_PATH_LEN 256

typedef struct {
    pid_t pid;
    char log_file[LOG_PATH_LEN];
    time_t last_update;
} MonitoredProcess;



MonitoredProcess processes[MAX_PROCESSES];
int process_count = 0;
int timeout =4; // Default timeout in seconds

// Function to check file modification time
time_t get_file_mod_time(const char *file_path) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) 
    {
        return -1; // Error checking file
    }
    return file_stat.st_mtime;
}

// Function to check if a process is still running
int is_process_alive(pid_t pid) {
    return kill(pid, 0) == 0; // Returns 0 if the process is alive, -1 if not
}

// Function to monitor processes
void monitor_processes() {
    while (1) {
        time_t current_time = time(NULL);

        for (int i = 0; i < process_count; i++) {
            // Check if the process is alive
            if (!is_process_alive(processes[i].pid)) {
                printf("Process %d has terminated. Killing all monitored processes...\n", processes[i].pid);

                // Kill all remaining processes
                for (int j = 0; j < process_count; j++) {
                    if (is_process_alive(processes[j].pid)) {
                        if (kill(processes[j].pid, SIGKILL) == -1) {
                            perror("Failed to terminate process");
                        } else {
                            printf("Process %d terminated successfully.\n", processes[j].pid);
                        }
                    }
                }

                // Exit the watchdog
                printf("Watchdog exiting.\n");
                sleep(10); // Wait for all processes to terminate
                exit(EXIT_SUCCESS);
            }
            
            // Check for log file inactivity (only if process is still alive)
            time_t mod_time = get_file_mod_time(processes[i].log_file);
            if (mod_time == -1) {
                fprintf(stderr, "Error reading log file: %s\n", processes[i].log_file);
                continue;
            }

            if (mod_time > processes[i].last_update) {
                processes[i].last_update = mod_time; // Update last known modification time
            } else if (current_time - processes[i].last_update > timeout) {
                // Kill the process if timeout exceeded
                printf("Process %d has not logged activity for %d seconds. Terminating...\n", processes[i].pid, timeout);
                if (kill(processes[i].pid, SIGKILL) == -1) {
                    perror("Failed to terminate process");
                } else {
                    printf("Process %d terminated successfully.\n", processes[i].pid);
                }
                // Remove the process from the monitoring list
                for (int j = i; j < process_count - 1; j++) {
                    processes[j] = processes[j + 1];
                }
                process_count--;
                i--; // Adjust index after removal
            }
        }

        sleep(1); // Check periodically
    }
}

// Function to add a process to monitor
void add_process(pid_t pid, const char *log_file) {
    if (process_count >= MAX_PROCESSES) {
        fprintf(stderr, "Maximum number of processes to monitor reached.\n");
        return;
    }

    processes[process_count].pid = pid;
    strncpy(processes[process_count].log_file, log_file, LOG_PATH_LEN - 1);
    processes[process_count].log_file[LOG_PATH_LEN - 1] = '\0';
    processes[process_count].last_update = get_file_mod_time(log_file);

    if (processes[process_count].last_update == -1) {
        fprintf(stderr, "Error reading log file: %s\n", log_file);
        return;
    }

    process_count++;
    printf("Added process %d to monitoring list (log file: %s).\n", pid, log_file);
}

// Main function
int main() {

    pid_t server_pid=get_pidd("BlackBoardServer");
    pid_t GameWindow=get_pidd("GameWindow");
    pid_t DroneDynamicsManager=get_pidd("DroneDynamicsManager");
    pid_t Targets_Generator=get_pidd("Targets_Generator");
    pid_t Obstcales_Generator=get_pidd("Obstacle_Generator");
    pid_t KeyboardManager=get_pidd("KeyboardManager");
    

    char* log_file = "../Logs/KeyBoard.log";
    char* log_file1 = "../Logs/BlackBoardServer.log";
    char* log_file2 ="../Logs/GameWindow.log";
    char* log_file3 = "../Logs/DroneDynamicsManager.log";
    char* log_file4 = "../Logs/TargetsGenerator.log";
    char* log_file5 = "../Logs/ObstacleGenerator.log";
    add_process(server_pid, log_file1);
    add_process(GameWindow, log_file2);
    add_process(DroneDynamicsManager, log_file3);
    add_process(Targets_Generator, log_file4);
    add_process(Obstcales_Generator, log_file5);
    add_process(KeyboardManager, log_file);

    // Start monitoring
    monitor_processes();

    return EXIT_SUCCESS;
}
