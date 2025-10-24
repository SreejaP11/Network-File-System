#include "client.h"

void handle_operations(int sock, char* client_operation)
{
    int error_code = 0;
    if(strncmp(client_operation, "READ", 4) == 0){
        char message_type;
        recv(sock, &message_type, sizeof(char), 0); // Receive the type first
        if(message_type == 'I'){
            SSInfo ss_info;
            recv(sock, &ss_info, MAX_LENGTH, 0);
            printf("%d, %s\n",ss_info.port, ss_info.ip);
            int client_sock = create_client_socket(ss_info.ip, ss_info.port);
            printf("Connected to storage server\n");
            send(client_sock, client_operation, MAX_LENGTH, 0);
            int num_of_packets;
            recv(client_sock, &num_of_packets, sizeof(int), 0);
            printf("%d\n", num_of_packets);
            char** file_content = (char**)malloc(MAX_PACKETS * sizeof(char*));
            for(int i = 0; i < MAX_LENGTH; i++){
                file_content[i] = (char*)malloc(MAX_LENGTH * sizeof(char));
            }
            for(int i = 0; i < num_of_packets; i++){
                recv(client_sock, file_content[i], MAX_LENGTH, 0);
                printf("%s", file_content[i]);
            }
            // printf("%s\n", file_content);
            printf("File content received successfully\n");
            for(int i = 0; i < MAX_LENGTH; i++){
                free(file_content[i]);
            }
            free(file_content);
            char ack_message[MAX_LENGTH];
            recv(client_sock, ack_message, MAX_LENGTH, 0);
            printf("ACK - %s\n", ack_message);
            close(client_sock);
            return;
        }
        else if(message_type == 'S'){
            char error_message[MAX_LENGTH];
            recv(sock, error_message, MAX_LENGTH, 0); // Receive the string message
            printf("Error: %s\n", error_message);
        }
        else {
            printf("Unknown message type received!\n");
        }
    }
    else if(strncmp(client_operation, "WRITE", 5) == 0){
        char message_type;
        recv(sock, &message_type, sizeof(char), 0); // Receive the type first
        if (message_type == 'I') {
            SSInfo ss_info;
            recv(sock, &ss_info, MAX_LENGTH, 0);
            printf("%d, %s\n", ss_info.port, ss_info.ip);
            int client_sock = create_client_socket(ss_info.ip, ss_info.port);
            printf("Connected to storage server\n");
            send(client_sock, client_operation, MAX_LENGTH, 0);

            // Allocate memory for file content packets
            char** file_data = (char**)malloc(MAX_PACKETS * sizeof(char*));
            for (int i = 0; i < MAX_PACKETS; i++) {
                file_data[i] = (char*)malloc(MAX_LENGTH * sizeof(char));
            }

            printf("Enter file content (type 'STOP' to finish):\n");
            int num_of_packets = 0;

            // Read and store packets until STOP is encountered or buffer is full
            while (num_of_packets < MAX_PACKETS && 
                fgets(file_data[num_of_packets], MAX_LENGTH, stdin) != NULL) {

                // Check if the current packet contains "STOP"
                if (strstr(file_data[num_of_packets], "STOP") != NULL) {
                    // Truncate the content at "STOP" and stop reading
                    char* stop_position = strstr(file_data[num_of_packets], "STOP");
                    *stop_position = '\0';  // Truncate at "STOP"
                    num_of_packets++;
                    break;
                }

                num_of_packets++;
            }

            // Send the number of packets to the server
            send(client_sock, &num_of_packets, sizeof(int), 0);

            // Send each packet to the server
            for (int i = 0; i < num_of_packets; i++) {
                send(client_sock, file_data[i], MAX_LENGTH, 0);
            }

            printf("File content sent successfully\n");

            char ack_message[MAX_LENGTH];
            recv(client_sock, ack_message, MAX_LENGTH, 0);
            printf("ACK - %s\n", ack_message);
            // Free allocated memory
            for (int i = 0; i < MAX_PACKETS; i++) {
                free(file_data[i]);
            }
            free(file_data);
            close(client_sock);
            return;
        }
        else if(message_type == 'S'){
            char error_message[MAX_LENGTH];
            recv(sock, error_message, MAX_LENGTH, 0); // Receive the string message
            printf("Error: %s\n", error_message);
        }  
        else {
            printf("Unknown message type received!\n");
        }
    }
    else if(strncmp(client_operation, "GET", 3) == 0){
        char message_type;
        recv(sock, &message_type, sizeof(char), 0); // Receive the type first
        if(message_type == 'I'){
            SSInfo ss_info;
            recv(sock, &ss_info, MAX_LENGTH, 0);
            printf("%d, %s\n",ss_info.port, ss_info.ip);
            int client_sock = create_client_socket(ss_info.ip, ss_info.port);
            printf("Connected to storage server\n");
            send(client_sock, client_operation, MAX_LENGTH, 0);
            char *path = malloc(strlen(client_operation) - 4 + 1);
            if (path == NULL) {
                printf("Memory allocation failed.\n");
                close(client_sock);
                return;
            }
            strcpy(path, &client_operation[4]);
            FolderInfo folder_info; 
            struct stat path_stat;
            stat(path, &path_stat);  // Get information about the path
            if (S_ISDIR(path_stat.st_mode)) {  // Directory
                recv(client_sock, &folder_info, sizeof(FolderInfo), 0);  // Receive folder info
                if (folder_info.valid == 0) {
                    printf("Error getting folder information\n");
                } else {
                    // Print the received folder information
                    printf("Received directory information successfully\n");
                    printf("Folder Name: %s\n", folder_info.folder_name);
                    printf("Size: %d bytes\n", folder_info.size);
                    printf("Permissions: %s\n", folder_info.Permissions);
                    printf("Last Modified Time: %s", folder_info.last_modified_time);
                    printf("Created Time: %s", folder_info.created_time);
                    printf("Parent Folder: %s\n", folder_info.parent_folder);
                    printf("Total Size in KB: %.2f KB\n", folder_info.total_size_of_folder_in_KB);
                    printf("Total Files: %d\n", folder_info.total_files);
                    printf("Total Folders: %d\n", folder_info.total_folders);
                }
            } 
            else if (S_ISREG(path_stat.st_mode)) {    // Getting information of file
                FileInfo file_info;
                recv(client_sock, &file_info, sizeof(FileInfo), 0);
                if(file_info.valid == 0){
                    printf("Error getting file information");
                }
                else{
                    printf("File Size: %ld bytes\n", file_info.file_size);
                    printf("File Permissions: %o\n", file_info.file_permissions);
                    printf("Last Access Time: %s", ctime(&file_info.last_access_time));
                    printf("Last Modification Time: %s", ctime(&file_info.last_modification_time));
                    printf("Last Status Change Time: %s", ctime(&file_info.last_status_change_time));
                    printf("File Extension: %s\n", file_info.file_extension);
                    printf("File Type: %s\n", file_info.file_type);
                    printf("File Owner: %d\n", file_info.file_owner);
                    printf("File Group: %d\n", file_info.file_group);
                    printf("File Inode Number: %lu\n", file_info.file_inode_number);
                    printf("File Device ID: %lx\n", file_info.file_device_id);
                    printf("Received file information successfully\n");
                }
            } 
            char ack_message[MAX_LENGTH];
            recv(client_sock, ack_message, MAX_LENGTH, 0);
            printf("ACK - %s\n", ack_message);
            close(client_sock);
            return;
        }
        else if(message_type == 'S'){
            char error_message[MAX_LENGTH];
            recv(sock, error_message, MAX_LENGTH, 0); // Receive the string message
            printf("Error: %s\n", error_message);
        }  
        else {
            printf("Unknown message type received!\n");
        }
    }
    else if(strncmp(client_operation, "LIST", 4) == 0){
        char message_type;
        recv(sock, &message_type, sizeof(char), 0); // Receive the type first
        if(message_type == 'I'){
            SSInfo ss_info;
            recv(sock, &ss_info, MAX_LENGTH, 0);
            printf("%d, %s\n", ss_info.port, ss_info.ip);
            int client_sock = create_client_socket(ss_info.ip, ss_info.port);
            printf("Connected to storage server\n");
            send(client_sock, client_operation, MAX_LENGTH, 0);
            
            ListFolder folder_list;
            recv(client_sock, &folder_list, MAX_LENGTH, 0);
            // Allocate memory for the folder paths list
            folder_list.paths_list = malloc(folder_list.count * sizeof(char*));
            if (folder_list.paths_list == NULL) {
                perror("Memory allocation failed for paths list");
                close(client_sock);
                return;
            }

            // Receive all paths and store them in the paths_list
            for (int i = 0; i < folder_list.count; i++) {
                folder_list.paths_list[i] = malloc(MAX_LENGTH * sizeof(char)); // Adjust size if necessary
                if (folder_list.paths_list[i] == NULL) {
                    perror("Memory allocation failed for path");
                    // Free previously allocated memory before returning
                    for (int j = 0; j < i; j++) {
                        free(folder_list.paths_list[j]);
                    }
                    free(folder_list.paths_list);
                    close(client_sock);
                    return;
                }
                recv(client_sock, folder_list.paths_list[i], MAX_LENGTH, 0);
            }
            printf("List of files and folders\n");
            // printf("%d\n", folder_list.count);
            // Print the paths
            for (int i = 0; i < folder_list.count; i++) {
                printf("%s\n", folder_list.paths_list[i]);
                free(folder_list.paths_list[i]); // Free each path after printing
            }
            free(folder_list.paths_list); // Free the paths list after usage
            char ack_message[MAX_LENGTH];
            recv(client_sock, ack_message, MAX_LENGTH, 0);
            printf("ACK - %s\n", ack_message);
            close(client_sock);
            return;
        }
        else if(message_type == 'S'){
            char error_message[MAX_LENGTH];
            recv(sock, error_message, MAX_LENGTH, 0); // Receive the string message
            printf("Error: %s\n", error_message);
        }  
        else {
            printf("Unknown message type received!\n");
        }
    }
    else if (strncmp(client_operation, "STREAM", 6) == 0) {
        char message_type;
        recv(sock, &message_type, sizeof(char), 0); // Receive the type first
        if(message_type == 'I'){
            SSInfo ss_info;
            recv(sock, &ss_info, MAX_LENGTH, 0);
            printf("%d, %s\n", ss_info.port, ss_info.ip);

            int client_sock = create_client_socket(ss_info.ip, ss_info.port);
            printf("Connected to storage server\n");
            send(client_sock, client_operation, MAX_LENGTH, 0);

            // Buffer to receive audio data
            char audio_buffer[MAX_FILE_LENGTH];
            ssize_t bytes_received;

            // Temporary file to store the received audio data
            FILE *temp_audio_file = fopen("temp_audio_file.wav", "wb");
            if (!temp_audio_file) {
                perror("Error creating temporary audio file");
                close(client_sock);
                return;
            }

            printf("Streaming audio...\n");
            while ((bytes_received = recv(client_sock, audio_buffer, sizeof(audio_buffer), 0)) > 0) {
                fwrite(audio_buffer, 1, bytes_received, temp_audio_file); // Write to temp file
            }

            if (bytes_received < 0) {
                perror("Error receiving audio data");
            }

            fclose(temp_audio_file); // Close the temp file
            printf("Audio streaming completed. Playing audio...\n");

            // Use exec to play the audio file using system's audio player
            if (fork() == 0) {
                // execlp("aplay", "aplay", "temp_audio_file.wav", (char *)NULL);
                execlp("ffplay", "ffplay", "-nodisp", "-autoexit", "temp_audio_file.wav", (char *)NULL);
                perror("Exec failed");
                exit(1);
            }

            wait(NULL); // Wait for the audio to finish playing
            remove("temp_audio_file.wav"); // Remove the temporary file
            char ack_message[MAX_LENGTH];
            recv(client_sock, ack_message, MAX_LENGTH, 0);
            printf("ACK %s\n", ack_message);
            close(client_sock);
            return;
        }
        else if(message_type == 'S'){
            char error_message[MAX_LENGTH];
            recv(sock, error_message, MAX_LENGTH, 0); // Receive the string message
            printf("Error: %s\n", error_message);
        }  
        else {
            printf("Unknown message type received!\n");
        }
    }
    else if(strncmp(client_operation, "CREATE", 6) == 0 || strncmp(client_operation, "DELETE", 6) == 0){
        char ack_message[MAX_LENGTH];
        recv(sock, ack_message, MAX_LENGTH, 0);
        printf("%s\n", ack_message);
        return;
    }
    else if(strcmp(client_operation, "ACCESS_LIST") == 0){
        char accessible_paths_str[2 * MAX_LENGTH];
        recv(sock, accessible_paths_str, 2 * MAX_LENGTH, 0);
        printf("Accessible paths:\n");
        char* tokens[MAX_PATHS];
        char* token = strtok(accessible_paths_str, "|");
        int token_count = 0;
        
        while (token != NULL && token_count < MAX_PATHS ) {
            tokens[token_count] = token;
            token_count++;
            token = strtok(NULL, "|");
        }
        // Allocate and copy each token into accessible_paths
        for (int i = 0; i < token_count; i++) {
            // new_server.accessible_paths[i] = malloc(strlen(tokens[i]) + 1);  // Allocate memory for each path
            printf("%s\n", tokens[i]);
            // strcpy(new_server.accessible_paths[i], tokens[i]);  // Copy the token (path) to accessible_paths
        }
        return;
    }
    else if(strncmp(client_operation, "COPY", 4) == 0){
        char ack_message[MAX_LENGTH];
        recv(sock, ack_message, MAX_LENGTH, 0);
        printf("%s\n", ack_message);
        return;
    }
    else{
        printf("Invalid operation\n");
        return;
    }
    recv(sock, &error_code, sizeof(int), 0);
    if(error_code != 0){
        printf("ERROR CODE: %d\n", error_code);
    }
}
