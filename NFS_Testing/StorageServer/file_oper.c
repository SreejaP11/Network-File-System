#include "file_oper.h"
#include "storage_server.h"
#include "headers.h"
#include "colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

extern AccessiblePaths accessible_paths;

// Function to validate if a path is accessible
int is_path_accessible(const char *path) {
    char resolved_path[PATH_MAX_LEN];
    // char *full_path = realpath(path, resolved_path);
    
    // if (full_path == NULL) return 0;
    if(path == NULL){
        return 0;
    }
    for (int i = 0; i < accessible_paths.path_count; i++) {
        if (strncmp(path, accessible_paths.paths[i], strlen(accessible_paths.paths[i])) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to create a directory
int create_directory(const char *path) 
{   
    // parse the last directory in the path and seprate it from the file name
    char *last_dir = strrchr(path, '/');
    char *dir = malloc(strlen(path) - strlen(last_dir) + 1);
    strncpy(dir, path, strlen(path) - strlen(last_dir));
    dir[strlen(path) - strlen(last_dir)] = '\0';
    printf("%s\n", dir);
    printf("%s\n", last_dir);
    printf("%s\n", path);
    if (!is_path_accessible(dir)) {
        printf("Path not accessible: %s\n", path);
        return -1;
    }
    
    int result = mkdir(path, 0777);
    if (result == 0 || errno == EEXIST) {
        return 0;
    }
    return -1;
}

// Function to create an empty file
int create_file(const char *path) 
{   

    // parse the last directory in the path and seprate it from the file name
    char *last_dir = strrchr(path, '/');
    char *dir = malloc(strlen(path) - strlen(last_dir) + 1);
    strncpy(dir, path, strlen(path) - strlen(last_dir));
    dir[strlen(path) - strlen(last_dir)] = '\0';
    if (!is_path_accessible(dir)) 
    {
        printf("Path not accessible: %s\n", dir);
        return -1;
    }
    
    
    FILE *file = fopen(path, "w");
    if (file == NULL) return -1;
    fclose(file);
    return 0;
}

// Function to recursively delete a directory
int delete_directory(const char *path) {
    if (!is_path_accessible(path)) {
        printf("Path not accessible: %s\n", path);
        return -1;
    }
    
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (d) {
        struct dirent *p;
        r = 0;

        while (!r && (p = readdir(d))) {
            int r2 = -1;
            char *buf;
            size_t len;

            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
                continue;

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);

            if (buf) {
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path, p->d_name);
                
                if (!stat(buf, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode))
                        r2 = delete_directory(buf);
                    else
                        r2 = unlink(buf);
                }
                free(buf);
            }
            r = r2;
        }
        closedir(d);
    }

    if (!r)
        r = rmdir(path);

    return r;
}

// Function to delete a file or directory
int delete_path(const char *path) {
    if (!is_path_accessible(path)) {
        printf("Path not accessible: %s\n", path);
        return -1;
    }
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) return -1;

    if (S_ISDIR(path_stat.st_mode)) {
        return delete_directory(path);
    } else {
        return remove(path);
    }
}

void handle_nm_command(int *sock) {
    int socket = *sock;
    free(sock); // Free the dynamically allocated memory

    char command[MAX_LENGTH] = {0};
    int read_size = recv(socket, command, MAX_LENGTH, 0);
    if (read_size <= 0) {
        // perror("NM disconnected or error receiving data");
        close(socket);
        return;
    }

    printf("Received operation from NM: %s\n", command);

    char response[MAX_LENGTH];
    char* path;
    if(strncmp(command, "CREATE", 6) == 0 || strncmp(command, "DELETE", 6) == 0){
        path = malloc(strlen(command) - 7 + 1);
        if (path == NULL) {
            printf("Memory allocation failed.\n");
            close(socket);
            return;
        }
        strcpy(path, &command[7]);
    }
    int result;
    if (strncmp(command, "CREATE", 6) == 0) {
        // Check if the path contains a '.' excluding the leading './'
        const char *path_without_leading_dot = path;
        if (strncmp(path, "./", 2) == 0) {
            path_without_leading_dot += 2; 
        }
        else if (strncmp(path, "../", 3) == 0) {
            path_without_leading_dot += 3; 
        }

        if (strchr(path_without_leading_dot, '.') != NULL) { // Dot found
            result = create_file(path);
            sprintf(response, "CREATE_FILE|%s|%s", path, result == 0 ? "SUCCESS" : "FAILED");
        } else { // No dot found
            result = create_directory(path);
            sprintf(response, "CREATE|%s|%s", path, result == 0 ? "SUCCESS" : "FAILED");
        }
        if(result == -1){
            error_code = 1;
        }
    }
    else if (strncmp(command, "DELETE", 6) == 0) {
        result = delete_path(path);
        sprintf(response, "DELETE|%s|%s", path, result == 0 ? "SUCCESS" : "FAILED");
    }
    else if(strncmp(command, "COPY", 4) == 0){
        char path_type[MAX_LENGTH];
        char src_path[MAX_LENGTH];
        char dest_path[MAX_LENGTH];
        recv(socket, path_type, MAX_LENGTH, 0);
        if(strcmp(path_type, "SOURCE") == 0){
            recv(socket, src_path, MAX_LENGTH, 0);
            FILE* file_pointer= fopen(src_path, "r");
            if(file_pointer == NULL){
                printf("Could not open file.\n");
                error_code = 2;
                send(socket, "Error opening file", MAX_LENGTH, 0);
                send(socket, &error_code, sizeof(int), 0);
                return;
            }
            else{
                printf("File opened successfully\n");
            }            
            char** file_data = (char**)malloc(MAX_PACKETS * sizeof(char*));
            for(int i = 0; i < MAX_LENGTH; i++){
                file_data[i] = (char*)malloc(MAX_LENGTH * sizeof(char));
            }
            int num_of_packets = 0;
            while(fgets(file_data[num_of_packets], MAX_LENGTH, file_pointer) != NULL) {
                num_of_packets++;
            }
            send(socket, &num_of_packets, sizeof(int), 0);
            for(int i =0 ; i < num_of_packets; i++){
                send(socket, file_data[i], MAX_LENGTH, 0);
            }
            fclose(file_pointer);
            for(int i = 0; i < MAX_LENGTH; i++){
                free(file_data[i]);
            }
            free(file_data);
            printf("Sent data successfully to NM\n");
            strcpy(response, "Sent to NM successfully");
        }
        else if(strcmp(path_type, "DESTINATION") == 0){
            recv(socket, dest_path, MAX_LENGTH, 0);
            int num_of_packets = 0;
            recv(socket, &num_of_packets, sizeof(int), 0);
            char** file_content = (char**)malloc(MAX_PACKETS * sizeof(char*));
            for(int i = 0; i < MAX_LENGTH; i++){
                file_content[i] = (char*)malloc(MAX_LENGTH * sizeof(char));
            }
            for(int i = 0; i < num_of_packets; i++){
                recv(socket, file_content[i], MAX_LENGTH, 0);
                printf("%s", file_content[i]);
            }
            printf("File content received successfully\n");
            size_t total_length = 0;
            for (int i = 0; i < num_of_packets; i++) {
                total_length += strlen(file_content[i]);
            }
            char* combined_content = (char*)malloc((total_length + 1) * sizeof(char)); 
            if (combined_content == NULL) {
                perror("Error allocating memory for combined content");
                exit(EXIT_FAILURE);
            }
            combined_content[0] = '\0'; // Initialize as an empty string
            for (int i = 0; i < num_of_packets; i++) {
                strcat(combined_content, file_content[i]); // Concatenate each packet
            }
            printf("%s\n", combined_content);
            if(write_file(dest_path, combined_content) == 0) { // Ensure read_file returns success/failure
                // send(socket, "Data from client written to file successfully", MAX_LENGTH, 0);
                printf("File content written successfully\n");
            } else {
                const char *error_msg = "Error writing to file";
                send(socket, error_msg, MAX_LENGTH, 0);
                return;
            }
            for(int i = 0; i < MAX_LENGTH; i++){
                free(file_content[i]);
            }
            free(file_content);
            strcpy(response, "Copied successfully");
        }
        // snprintf(response,"Copied %s to %s successfully", src_path, dest_path);
    }
    send(socket, response, MAX_LENGTH, 0);
    printf("Sent response: %s\n", response);
    send(socket, &error_code, sizeof(int), 0);
}