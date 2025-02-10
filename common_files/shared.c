#include"shared.h"
#include<errno.h>






int create_and_open_fifo(const char *t, int identifier, int flags) {
    char fifo_name[256];
    snprintf(fifo_name, sizeof(fifo_name), t, identifier);

    if (mkfifo(fifo_name, 0666) < 0 && errno != EEXIST) {
        perror("Failed to create FIFO");
        exit(1);
    }

    int fd = open(fifo_name, flags);
    if (fd < 0) {
        perror("Failed to open FIFO");
        exit(1);
    }

    return fd;
}


void unlink_fifo(const char *t, int identifier) {
    char fifo_name[256];
    snprintf(fifo_name, sizeof(fifo_name), t, identifier);

    if (unlink(fifo_name) < 0) {
        perror("Failed to unlink FIFO");
    } else {
        printf("FIFO %s successfully unlinked.\n", fifo_name);
    }
}

ServerState initialize_server_state(int n_obstacles, int n_targets) {
    return (ServerState){
        .drone_x = 10,
        .drone_y = 7,
        .input_x_force = 0,
        .input_y_force = 0,
        .resultant_force_x = 0,
        .resultant_force_y = 0,
        .velocity_x = 0,
        .velocity_y = 0,
        .num_obstacles = n_obstacles,
        .num_targets = n_targets,
    };
}





pid_t get_pidd(const char *program_name) {
    char line[256];
    // Construct the command to get the PID of the process
    char command[256];
    snprintf(command, sizeof(command), "pidof %s", program_name);

    // Open the command for reading
    FILE *cmd = popen(command, "r");
    if (cmd == NULL) {
        perror("Failed to run pidof");
        return -1;
    }

    // Read the output (the PID) from the command
    if (fgets(line, 256, cmd) != NULL) {
        // Convert the string to pid_t (unsigned long) and return the PID
        pid_t pid = strtoul(line, NULL, 10);
        pclose(cmd);
        return pid;
    } else {
        // If no output (process not found), close and return -1
        pclose(cmd);
        return -1;
    }
}

int get_int_from_json(const char *filename, const char *key, int *value) {
    // Open the file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open JSON file");
        return -1;
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Handle empty files
    if (length == 0) {
        fprintf(stderr, "Error: JSON file is empty\n");
        fclose(file);
        return -1;
    }

    // Allocate memory for the file content
    char *data = (char *)malloc(length + 1);
    if (data == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return -1;
    }

    // Read the file content
    size_t bytes_read = fread(data, 1, length, file);
    if (bytes_read != length) {
        perror("Failed to read JSON file");
        free(data);
        fclose(file);
        return -1;
    }
    data[length] = '\0'; // Null-terminate the string
    fclose(file);

    // Parse the JSON data
    cJSON *json = cJSON_Parse(data);
    free(data); // Free the file content buffer
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error parsing JSON: %s\n", error_ptr);
        }
        return -1;
    }

    // Retrieve the value associated with the key
    cJSON *json_value = cJSON_GetObjectItemCaseSensitive(json, key);
    if (json_value == NULL || !cJSON_IsNumber(json_value)) {
        fprintf(stderr, "Key '%s' not found or is not a number\n", key);
        cJSON_Delete(json);
        return -1;
    }

    // Store the retrieved value
    *value = json_value->valueint; 
    cJSON_Delete(json);
    return 0;
}











int write_int_to_json(const char *filename, const char *key, int value) {
    // Open the file
    FILE *file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Failed to open JSON file");
        return -1;
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Handle empty files
    if (length == 0) {
        fprintf(stderr, "Error: JSON file is empty\n");
        fclose(file);
        return -1;
    }

    // Allocate memory for the file content
    char *data = (char *)malloc(length + 1);
    if (data == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return -1;
    }

    // Read the file content
    size_t bytes_read = fread(data, 1, length, file);
    if (bytes_read != length) {
        perror("Failed to read JSON file");
        free(data);
        fclose(file);
        return -1;
    }
    data[length] = '\0'; // Null-terminate the string

    // Parse the JSON data
    cJSON *json = cJSON_Parse(data);
    free(data); // Free the file content buffer
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error parsing JSON: %s\n", error_ptr);
        }
        fclose(file);
        return -1;
    }

    // Update the value associated with the key
    cJSON *json_value = cJSON_GetObjectItemCaseSensitive(json, key);
    if (json_value == NULL) {
        // If the key does not exist, create it
        json_value = cJSON_CreateNumber(value);
        cJSON_AddItemToObject(json, key, json_value);
    } else {
        // If the key exists, update its value
        if (!cJSON_IsNumber(json_value)) {
            fprintf(stderr, "Key '%s' is not a number\n", key);
            cJSON_Delete(json);
            fclose(file);
            return -1;
        }
        json_value->valueint = value;
    }

    // Convert the JSON object to a string
    char *updated_data = cJSON_Print(json);
    cJSON_Delete(json);
    if (updated_data == NULL) {
        perror("Failed to print JSON");
        fclose(file);
        return -1;
    }

    // Write the updated JSON data back to the file
    fseek(file, 0, SEEK_SET);
    if (fwrite(updated_data, 1, strlen(updated_data), file) != strlen(updated_data)) {
        perror("Failed to write JSON file");
        free(updated_data);
        fclose(file);
        return -1;
    }
    free(updated_data);

    // Truncate the file to the new length
    if (ftruncate(fileno(file), strlen(updated_data)) != 0) {
        perror("Failed to truncate JSON file");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}
