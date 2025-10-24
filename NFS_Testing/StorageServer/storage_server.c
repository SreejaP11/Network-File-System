#include "storage_server.h"
#include "socket_utils.h" 
#include "file_oper.h"
#include "colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "headers.h"
#include <fcntl.h>

int error_code = 0;
void initialize_storage_server(const char *nm_ip, int nm_port, char* ss_ip, int client_port, int server_port, AccessiblePaths *accessible_paths) {
    printf("Initializing Storage Server...\n");
    register_with_naming_server(nm_ip, nm_port, ss_ip, client_port, server_port, accessible_paths);
    int client_socket_fd = create_server_socket(client_port, 5);
    printf(GREEN "%d\n" RESET, server_port);
    int nm_socket_fd = create_server_socket(server_port, 5);  
    if (client_socket_fd < 0 || nm_socket_fd < 0) {
        perror("Failed to set up sockets");
        exit(EXIT_FAILURE);
    }

    printf("Storage Server ready to accept connections on Client Port %d and NM Port %d\n", client_port, server_port);
    pthread_t nm_thread, client_thread;
    pthread_create(&nm_thread, NULL, (void *)handle_naming_server_requests, &nm_socket_fd);
    pthread_create(&client_thread, NULL, (void *)handle_client_requests, &client_socket_fd);
    pthread_join(nm_thread, NULL);
    pthread_join(client_thread, NULL);
}

void register_with_naming_server(const char *nm_ip, int nm_port, char* ss_ip, int client_port, int server_port, AccessiblePaths *accessible_paths) {
    printf(GREEN"%d\n"RESET,nm_port);
    int sock = create_client_socket(nm_ip, nm_port);
    if (sock < 0) {
        perror("Failed to connect to Naming Server");
        exit(EXIT_FAILURE);
    }
    send(sock, "STORAGE_SERVER", MAX_LENGTH, 0);
    StorageServerInfo ss_info;
    strcpy(ss_info.ip, ss_ip);
    ss_info.nm_port = server_port;
    ss_info.client_port = client_port;
    char reg_message[8000] = {0};
    for(int i=0;i<accessible_paths->path_count;i++){
        strcat(reg_message,"|");
        strcat(reg_message,accessible_paths->paths[i]);
    }
    strcpy(ss_info.accessible_paths, reg_message);
    printf("%s\n", ss_info.accessible_paths);
    send(sock, &ss_info, MAX_SS_LENGTH, 0);
    printf("Registered with Naming Server: IP %s, Client Port %d, Server Port %d\n", ss_ip, client_port, server_port);
    close(sock);
}

void handle_naming_server_requests(int *server_socket_ptr) {
    int server_socket = *server_socket_ptr;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    printf("Listening for Naming Server requests on NM Port...\n");
    char request[MAX_LENGTH] = {0};
    while (1) {
        int nm_conn = accept(server_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (nm_conn < 0) {
            perror("Failed to accept Naming Server connection");
            return;
        }

        pthread_t ss_thread;
        int *client_sock_ptr = malloc(sizeof(int));
        if (client_sock_ptr == NULL) {
            perror("Memory allocation failed for client socket pointer");
            close(nm_conn);
            return;
        }
        *client_sock_ptr = nm_conn;
        // handle_single_client(client_sock_ptr);
        pthread_create(&ss_thread, NULL, (void *)handle_nm_command, client_sock_ptr);
        pthread_detach(ss_thread); // Detach thread to avoid memory leaks
    } 
}

void handle_client_requests(int *client_socket_ptr) {
    int server_socket = *client_socket_ptr;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    printf("Listening for client requests on Client Port...\n");
    while (1){
        int client_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_socket < 0) {
            perror("Failed to accept client connection");
            return; // Keep the server running
        }
        printf("Client connected: %s:%d\n", inet_ntoa(address.sin_addr), address.sin_port);

        pthread_t client_thread;
        int *client_sock_ptr = malloc(sizeof(int));
        if (client_sock_ptr == NULL) {
            perror("Memory allocation failed for client socket pointer");
            close(client_socket);
            return;
        }
        *client_sock_ptr = client_socket;
        // handle_single_client(client_sock_ptr);
        pthread_create(&client_thread, NULL, (void *)handle_single_client, client_sock_ptr);
        pthread_detach(client_thread); // Detach thread to avoid memory leaks
    }
}

void handle_single_client(int *client_socket_ptr) {
    int client_socket = *client_socket_ptr;
    free(client_socket_ptr); // Free the dynamically allocated memory

    char client_operation[MAX_LENGTH] = {0};
    int read_size = recv(client_socket, client_operation, MAX_LENGTH, 0);
    if (read_size <= 0) {
        perror("Client disconnected or error receiving data");
        close(client_socket);
        return;
    }

    printf("Received operation from client: %s\n", client_operation);

    if(strncmp(client_operation, "READ", 4) == 0){
        char *path = malloc(strlen(client_operation) - 5 + 1);
        if (path == NULL) {
            printf("Memory allocation failed.\n");
            close(client_socket);
            return;
        }
        strcpy(path, &client_operation[5]);
        read_file(path, client_socket);
        send(client_socket, "Read completed successfully", MAX_LENGTH, 0);
        free(path);
    }
    else if(strncmp(client_operation, "WRITE", 5) == 0){
        char *path = malloc(strlen(client_operation) - 6 + 1);
        if (path == NULL) {
            printf("Memory allocation failed.\n");
            error_code = 5;
            send(client_socket, &error_code, sizeof(int), 0);
            close(client_socket);
            return;
        }
        strcpy(path, &client_operation[6]);
        int num_of_packets;
        recv(client_socket, &num_of_packets, sizeof(int), 0);
        // printf("%d\n", num_of_packets);
        char** file_content = (char**)malloc(MAX_PACKETS * sizeof(char*));
        for(int i = 0; i < MAX_LENGTH; i++){
            file_content[i] = (char*)malloc(MAX_LENGTH * sizeof(char));
        }
        for(int i = 0; i < num_of_packets; i++){
            recv(client_socket, file_content[i], MAX_LENGTH, 0);
            // printf("%s", file_content[i]);
        }
        printf("File content received successfully\n");
        // Combine all packets into a single buffer
        size_t total_length = 0;
        for (int i = 0; i < num_of_packets; i++) {
            total_length += strlen(file_content[i]);
        }

        char* combined_content = (char*)malloc((total_length + 1) * sizeof(char)); // +1 for null terminator
        if (combined_content == NULL) {
            perror("Error allocating memory for combined content");
            exit(EXIT_FAILURE);
        }

        combined_content[0] = '\0'; // Initialize as an empty string
        for (int i = 0; i < num_of_packets; i++) {
            strcat(combined_content, file_content[i]); // Concatenate each packet
        }
        // char file_content[MAX_LENGTH] = {0};
        // recv(client_socket, file_content, MAX_LENGTH, 0);
        if(write_file(path, combined_content) == 0) { // Ensure read_file returns success/failure
            send(client_socket, "Data from client written to file successfully", MAX_FILE_LENGTH, 0);
            printf("File content written successfully\n");
        } else {
            const char *error_msg = "Error writing to file";
            send(client_socket, error_msg, MAX_LENGTH, 0);
            error_code = 10;
            send(client_socket, &error_code, sizeof(int), 0);
        }
        for(int i = 0; i < MAX_LENGTH; i++){
            free(file_content[i]);
        }
        free(file_content);
        free(path);
    }
    else if(strncmp(client_operation, "GET", 3) == 0){
        char *path = malloc(strlen(client_operation) - 4 + 1);
        if (path == NULL) {
            printf("Memory allocation failed.\n");
            error_code = 5;
            send(client_socket, &error_code, sizeof(int), 0);
            close(client_socket);
            return;
        }
        strcpy(path, &client_operation[4]);
        struct stat path_stat;
        stat(path, &path_stat);  // Get information about the path
        if (S_ISDIR(path_stat.st_mode)) {            // Getting information of directory
            FolderInfo folder_info = additional_information_of_folder(path);
            printf("Folder Name: %s\n", folder_info.folder_name);
            printf("Size: %d bytes\n", folder_info.size);
            printf("Permissions: %s\n", folder_info.Permissions);
            printf("Last Modified Time: %s", folder_info.last_modified_time);
            printf("Created Time: %s", folder_info.created_time);
            printf("Parent Folder: %s\n", folder_info.parent_folder);
            printf("Total Size in KB: %.2f KB\n", folder_info.total_size_of_folder_in_KB);
            printf("Total Files: %d\n", folder_info.total_files);
            printf("Total Folders: %d\n", folder_info.total_folders);
            send(client_socket, &folder_info, sizeof(FolderInfo), 0);
            printf("Sent directory information successfully\n");
            send(client_socket, "Sent directory information successfully", MAX_LENGTH, 0);
        } else if (S_ISREG(path_stat.st_mode)) {
            // If the path is a regular file
            FileInfo file_info = additional_information_of_file(path);
            send(client_socket, &file_info, sizeof(FileInfo), 0);
            printf("Sent file information successfully\n");
            send(client_socket, "Sent file information successfully", MAX_LENGTH, 0);
        } else {
            // If the path is neither a file nor a directory
            send(client_socket, "Invalid path", MAX_LENGTH, 0);
            printf("The path is neither a file nor a directory\n");
        }
        free(path);
    }
    else if(strncmp(client_operation, "LIST", 4) == 0){
        char *path = malloc(strlen(client_operation) - 5 + 1);
        if (path == NULL) {
            printf("Memory allocation failed.\n");
            error_code = 5;
            send(client_socket, &error_code, sizeof(int), 0);
            close(client_socket);
            return;
        }
        strcpy(path, &client_operation[5]);
        char** list_folders;
        int count = 0;
        list_folders = listing_all_files_and_folders(path, &count);
        ListFolder folder_list;
        folder_list.count = count;
        folder_list.paths_list = list_folders;
        send(client_socket, &folder_list, MAX_LENGTH, 0);
        for (int i = 0; i < count; i++) {
            send(client_socket, list_folders[i], MAX_LENGTH, 0); // Send each path
        }
        send(client_socket, "Sent list of files and folders successfully", MAX_LENGTH, 0);
    }
    else if (strncmp(client_operation, "STREAM", 6) == 0) {
        char *path = malloc(strlen(client_operation) - 7 + 1);
        if (path == NULL) {
            printf("Memory allocation failed.\n");
            error_code = 5;
            send(client_socket, &error_code, sizeof(int), 0);
            close(client_socket);
            return;
        }
        strcpy(path, &client_operation[7]);
        int result = streaming_audio_file(path, client_socket);
        send(client_socket, "Audio streaming completed", MAX_LENGTH, 0);
    }
    else {
        const char *error_msg = "Invalid operation";
        send(client_socket, error_msg, MAX_FILE_LENGTH, 0);
    }

    close(client_socket); // Ensure the socket is closed
    printf("Client connection closed\n");
}
