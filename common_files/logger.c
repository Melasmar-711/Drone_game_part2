#include "logger.h"

// Helper function to get the current timestamp as a string
const char* get_current_time() {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    static char time_buffer[20];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    return time_buffer;
}

// Function to write the log message to a file
void log_message(const char *log_file, LogLevel level, const char *message) {
    FILE *log_fp = fopen(log_file, "a");  // Open log file in append mode 
    if (log_fp == NULL) {
        perror("Failed to open log file");
        return;
    }

    // Get the current time
    const char *timestamp = get_current_time();

    // Map LogLevel enum to string
    const char *level_str;
    switch (level) {
        case INFO: level_str = "INFO"; break;
        case WARNING: level_str = "WARNING"; break;
        case ERROR: level_str = "ERROR"; break;
        default: level_str = "UNKNOWN"; break;
    }

    // Format the log message
    fprintf(log_fp, "[%s] [%s] %s\n", timestamp, level_str, message);
    fclose(log_fp);

    retainLastNLines(log_file, MAX_LINES);  // Retain only the last 5 lines
}





void retainLastNLines(const char *filename, int n) {
    char buffer[MAX_LINES][MAX_LINE_LENGTH];  // Buffer to store lines
    int lineCount = 0;

    // Open the file for reading
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Read all lines into the buffer
    while (fgets(buffer[lineCount], MAX_LINE_LENGTH, file) != NULL) {
        lineCount++;
        if (lineCount >= MAX_LINES) {
            // Shift lines up to make room for new lines
            for (int i = 1; i < MAX_LINES; i++) {
                strcpy(buffer[i - 1], buffer[i]);
            }
            lineCount--;
        }
    }
    fclose(file);

    // Open the file for writing (truncates the file)
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Write the last N lines to the file
    int startLine = (lineCount > n) ? (lineCount - n) : 0;
    for (int i = startLine; i < lineCount; i++) {
        fprintf(file, "%s", buffer[i]);
    }

    fclose(file);
    //printf("Retained the last %d lines of the log file.\n", n);
}