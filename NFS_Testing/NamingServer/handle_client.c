#include "naming_server.h"
#include "socket_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void handle_client_request(int client_socket) 
{
    FILE *file = fopen(FILENAME, "a");
    char client_request[BUFFER_SIZE] = {0};
    int read_size = recv(client_socket, client_request, MAX_LENGTH, 0);
    if (read_size > 0) {
        client_request[read_size] = '\0';
        printf("Received client request: %s\n", client_request);
        if(strncmp(client_request, "READ", 4) == 0) {
            char* path = malloc(strlen(client_request) - 5 + 1); 
            if (path == NULL) {
                printf("Memory allocation failed.\n");
                error_code = 5;
                send(client_socket, &error_code, sizeof(int), 0);
                fclose(file);
                return;
            }
            strcpy(path, &client_request[5]);
            SSInfo_Client ss_info;
            // StorageServer* result = (StorageServer*)malloc(sizeof(StorageServer));
            StorageServer* result = NULL;
            for(int i = 0; i < server_count; i++){
                // print_all_paths(storage_servers[i].path_trie);
                result = search_path(storage_servers[i].path_trie, path);
                if(result != NULL){
                    char type = 'I'; // 'I' for SSInfo
                    send(client_socket, &type, sizeof(char), 0); // Send the type first
                    ss_info.client_port = storage_servers[i].ssinfo.client_port;
                    strcpy(ss_info.ip, storage_servers[i].ssinfo.ss_ip);
                    printf("%d, %s\n", ss_info.client_port, ss_info.ip);
                    send(client_socket, &ss_info, MAX_LENGTH, 0);
                    free(path);
                    fprintf(file, "Client request: %s - SS Port: %d, IP: %s\n", client_request, ss_info.client_port, ss_info.ip);
                    fclose(file);
                    return;
                }
            }
            if(result == NULL){
                printf("Path not accessible\n");
                char type = 'S'; // 'S' for string
                send(client_socket, &type, sizeof(type), 0); // Send the type first
                send(client_socket, "Path not accessible", MAX_LENGTH, 0);
                error_code = 1;
                send(client_socket, &error_code, sizeof(int), 0);
            }
            free(path);
        }
        else if(strncmp(client_request, "WRITE", 5) == 0) {
            char* path = malloc(strlen(client_request) - 6 + 1); 
            if (path == NULL) {
                printf("Memory allocation failed.\n");
                error_code = 5;
                send(client_socket, &error_code, sizeof(int), 0);
                fclose(file);
                return;
            }
            strcpy(path, &client_request[6]);
            SSInfo_Client ss_info;
            StorageServer* result = NULL;
            char *last_dir = strrchr(path, '/');
            char *dir = malloc(strlen(path) - strlen(last_dir) + 1);
            strncpy(dir, path, strlen(path) - strlen(last_dir));
            dir[strlen(path) - strlen(last_dir)] = '\0';
            // printf("%s\n", dir);
            int i;
            for(i = 0; i < server_count; i++){
                result = search_path(storage_servers[i].path_trie, dir);
                if(result != NULL){
                    char type = 'I'; // 'I' for SSInfo
                    send(client_socket, &type, sizeof(char), 0); // Send the type first
                    ss_info.client_port = storage_servers[i].ssinfo.client_port;
                    strcpy(ss_info.ip, storage_servers[i].ssinfo.ss_ip);
                    printf("%d, %s\n", ss_info.client_port, ss_info.ip);
                    send(client_socket, &ss_info, MAX_LENGTH, 0);
                    break;
                }
            }
            if(result == NULL){
                printf("Path not accessible\n");
                char type = 'S'; // 'S' for string
                send(client_socket, &type, sizeof(type), 0); // Send the type first
                send(client_socket, "Path not accessible", MAX_LENGTH, 0);
                error_code = 1;
                send(client_socket, &error_code, sizeof(int), 0);
                fclose(file);
                return;
            }
            result = NULL;
            for(int j = 0; j < server_count; j++){
                result = search_path(storage_servers[j].path_trie, path);
                if(result != NULL){
                    fclose(file);
                    return;
                }
            }
            if(result == NULL){
                storage_servers[i].accessible_paths[storage_servers[i].path_count] = malloc(MAX_LENGTH * sizeof(char));
                strcpy(storage_servers[i].accessible_paths[storage_servers[i].path_count++], path);
                insert_path(storage_servers[i].path_trie, path, &storage_servers[i]);
            }
            fprintf(file, "Client request: %s - SS Port: %d, IP: %s\n", client_request, ss_info.client_port, ss_info.ip);
            free(path);
        }
        else if(strncmp(client_request, "GET", 3) == 0){
            char* path = malloc(strlen(client_request) - 4 + 1); 
            if (path == NULL) {
                printf("Memory allocation failed.\n");
                error_code = 5;
                send(client_socket, &error_code, sizeof(int), 0);
                fclose(file);
                return;
            }
            strcpy(path, &client_request[4]);
            SSInfo_Client ss_info;
            StorageServer* result = NULL;
            // print_all_paths(storage_servers[0].path_trie);
            for(int i = 0; i < server_count; i++){
                // print_all_paths(storage_servers[i].path_trie);
                result = search_path(storage_servers[i].path_trie, path);
                if(result != NULL){
                    char type = 'I'; // 'I' for SSInfo
                    send(client_socket, &type, sizeof(char), 0); // Send the type first
                    ss_info.client_port = storage_servers[i].ssinfo.client_port;
                    strcpy(ss_info.ip, storage_servers[i].ssinfo.ss_ip);
                    printf("%d, %s\n", ss_info.client_port, ss_info.ip);
                    send(client_socket, &ss_info, MAX_LENGTH, 0);
                    free(path);
                    fprintf(file, "Client request: %s - SS Port: %d, IP: %s\n", client_request, ss_info.client_port, ss_info.ip);
                    fclose(file);
                    return;
                }
            }
            if(result == NULL){
                printf("Path not accessible\n");
                char type = 'S'; // 'S' for string
                send(client_socket, &type, sizeof(type), 0); // Send the type first
                send(client_socket, "Path not accessible", MAX_LENGTH, 0);
                error_code = 1;
                send(client_socket, &error_code, sizeof(int), 0);
            }
            free(path);
        }
        else if(strncmp(client_request, "LIST", 4) == 0){
            char* path = malloc(strlen(client_request) - 5 + 1); 
            if (path == NULL) {
                printf("Memory allocation failed.\n");
                error_code = 5;
                send(client_socket, &error_code, sizeof(int), 0);
                fclose(file);
                return;
            }
            strcpy(path, &client_request[5]);
            SSInfo_Client ss_info;
            StorageServer* result = NULL;
            for(int i = 0; i < server_count; i++){
                result = search_path(storage_servers[i].path_trie, path);
                if(result != NULL){
                    char type = 'I'; // 'I' for SSInfo
                    send(client_socket, &type, sizeof(char), 0); // Send the type first
                    ss_info.client_port = storage_servers[i].ssinfo.client_port;
                    strcpy(ss_info.ip, storage_servers[i].ssinfo.ss_ip);
                    printf("%d, %s\n", ss_info.client_port, ss_info.ip);
                    send(client_socket, &ss_info, MAX_LENGTH, 0);
                    free(path);
                    fprintf(file, "Client request: %s - SS Port: %d, IP: %s\n", client_request, ss_info.client_port, ss_info.ip);
                    fclose(file);
                    return;
                }
            }
            if(result == NULL){
                printf("Path not accessible\n");
                char type = 'S'; // 'S' for string
                send(client_socket, &type, sizeof(type), 0); // Send the type first
                send(client_socket, "Path not accessible", MAX_LENGTH, 0);
                error_code = 1;
                send(client_socket, &error_code, sizeof(int), 0);
            }
            free(path);
        }
        else if(strncmp(client_request, "STREAM", 6) == 0) {
            char* path = malloc(strlen(client_request) - 7 + 1); 
            if (path == NULL) {
                printf("Memory allocation failed.\n");
                error_code = 5;
                send(client_socket, &error_code, sizeof(int), 0);
                fclose(file);
                return;
            }
            strcpy(path, &client_request[7]);
            SSInfo_Client ss_info;
            StorageServer* result = NULL;
            for(int i = 0; i < server_count; i++){
                result = search_path(storage_servers[i].path_trie, path);
                if(result != NULL){
                    char type = 'I'; // 'I' for SSInfo
                    send(client_socket, &type, sizeof(char), 0); // Send the type first
                    ss_info.client_port = storage_servers[i].ssinfo.client_port;
                    strcpy(ss_info.ip, storage_servers[i].ssinfo.ss_ip);
                    printf("%d, %s\n", ss_info.client_port, ss_info.ip);
                    send(client_socket, &ss_info, MAX_LENGTH, 0);
                    free(path);
                    fprintf(file, "Client request: %s - SS Port: %d, IP: %s\n", client_request, ss_info.client_port, ss_info.ip);
                    fclose(file);
                    return;
                }
            }
            if(result == NULL){
                printf("Path not accessible\n");
                char type = 'S'; // 'S' for string
                send(client_socket, &type, sizeof(type), 0); // Send the type first
                send(client_socket, "Path not accessible", MAX_LENGTH, 0);
                error_code = 1;
                send(client_socket, &error_code, sizeof(int), 0);
            }
            free(path);
        }
        else if(strncmp(client_request, "CREATE", 6) == 0 || strncmp(client_request, "DELETE", 6) == 0){
            char* path = malloc(strlen(client_request) - 7 + 1); 
            if (path == NULL) {
                printf("Memory allocation failed.\n");
                error_code = 5;
                send(client_socket, &error_code, sizeof(int), 0);
                fclose(file);
                return;
            }
            strcpy(path, &client_request[7]);
            StorageServer* req_ss = NULL;
            for(int i = 0; i < server_count; i++){
                if(strncmp(client_request, "CREATE", 6) == 0){
                    char *last_dir = strrchr(path, '/');
                    char *dir = malloc(strlen(path) - strlen(last_dir) + 1);
                    strncpy(dir, path, strlen(path) - strlen(last_dir));
                    dir[strlen(path) - strlen(last_dir)] = '\0';
                    printf("%s\n", dir);
                    req_ss = search_path(storage_servers[i].path_trie, dir);
                }
                else{
                    req_ss = search_path(storage_servers[i].path_trie, path);
                }
                if(req_ss != NULL){
                    storage_servers[i].server_socket = create_client_socket(storage_servers[i].ssinfo.ss_ip, storage_servers[i].ssinfo.server_port);
                    send(storage_servers[i].server_socket, client_request, MAX_LENGTH, 0);
                    printf("Request sent successfully");
                    fprintf(file, "Client request: %s - Received ack from ss port: %d, ip: %s\n", client_request, storage_servers[i].ssinfo.server_port, storage_servers[i].ssinfo.ss_ip);
                    char ack_message[MAX_LENGTH];
                    recv(storage_servers[i].server_socket, ack_message, MAX_LENGTH, 0);
                    printf("%s\n", ack_message);
                    if(strncmp(client_request, "CREATE", 6) == 0){
                        storage_servers[i].accessible_paths[storage_servers[i].path_count] = malloc(MAX_LENGTH * sizeof(char));
                        strcpy(storage_servers[i].accessible_paths[storage_servers[i].path_count++], path);
                        insert_path(storage_servers[i].path_trie, path, &storage_servers[i]);
                        // print_all_paths(storage_servers[i].path_trie);
                    }
                    else{
                        int j;
                        for(j = 0; j < storage_servers[i].path_count; j++){
                            if(strcmp(storage_servers[i].accessible_paths[j], path) == 0){
                                break;
                            }
                        }
                        for(int k = j; k < storage_servers[i].path_count - 1; k++){
                            strcpy(storage_servers[i].accessible_paths[k], storage_servers[i].accessible_paths[k + 1]);
                        }
                        // free(storage_servers[i].accessible_paths[storage_servers[i].path_count - 1]);
                        storage_servers[i].accessible_paths[storage_servers[i].path_count - 1] = "";
                        storage_servers[i].path_count--;
                        remove_path(storage_servers[i].path_trie, path);
                        // print_all_paths(storage_servers[i].path_trie);
                    }
                    send(client_socket, ack_message, MAX_LENGTH, 0);
                    printf("Sent acknowledgement successfully\n");
                    fclose(file);
                    return;
                }
            }
            if(req_ss == NULL){
                printf("Path not accessible\n");
                send(client_socket, "Path not accessible", MAX_LENGTH, 0);
                error_code = 1;
                send(client_socket, &error_code, sizeof(int), 0);
                fclose(file);
                return;
            }
        }
        else if(strcmp(client_request, "ACCESS_LIST") == 0){
            char accessible_concat[8 * BUF_LEN] = {0};
            for(int i = 0; i < server_count; i++){
                for(int j = 0;j < storage_servers[i].path_count; j++){
                    strcat(accessible_concat,"|");
                    strcat(accessible_concat,storage_servers[i].accessible_paths[j]);
                }
                // printf("%s\n", accessible_concat);
            }
            send(client_socket, accessible_concat, 8 * BUF_LEN, 0);
        }
        else if(strncmp(client_request, "COPY", 4) == 0){
            char src_path[256], dest_path[256]; // Adjust size based on your path length needs
            char *token = strtok(client_request, " ");
            if (token == NULL || strcmp(token, "COPY") != 0) {
                printf("Invalid command format\n");
                fclose(file);
                return;
            }
            token = strtok(NULL, " ");
            if (token == NULL) {
                printf("Missing source path\n");
                fclose(file);
                return;
            }
            strcpy(src_path, token);
            token = strtok(NULL, " ");
            if (token == NULL) {
                printf("Missing destination path\n");
                fclose(file);
                return;
            }
            strcpy(dest_path, token);
            // printf("%s\n", src_path);
            // printf("%s\n", dest_path);
            int src_port, dest_port;
            char src_ip[32], dest_ip[32];
            StorageServer* result = NULL;
            for(int i = 0; i < server_count; i++){
                result = search_path(storage_servers[i].path_trie, src_path);
                if(result != NULL){
                    src_port = storage_servers[i].ssinfo.server_port;
                    strcpy(src_ip, storage_servers[i].ssinfo.ss_ip);
                    break;
                }
            }
            if(result == NULL){
                printf("Source path not accessible\n");
                send(client_socket, "Source path not accessible", MAX_LENGTH, 0);
                error_code = 1;
                send(client_socket, &error_code, sizeof(int), 0);
                fclose(file);
                return;
            }
            result = NULL;
            int ss_no;
            for(ss_no = 0; ss_no < server_count; ss_no++){
                result = search_path(storage_servers[ss_no].path_trie, dest_path);
                if(result != NULL){
                    dest_port = storage_servers[ss_no].ssinfo.server_port;
                    strcpy(dest_ip, storage_servers[ss_no].ssinfo.ss_ip);
                    break;
                }
            }
            if(result == NULL){
                printf("Destination path not accessible\n");
                send(client_socket, "Destination path not accessible", MAX_LENGTH, 0);
                error_code = 1;
                send(client_socket, &error_code, sizeof(int), 0);
                fclose(file);
                return;
            }
            int src_socket = create_client_socket(src_ip, src_port);
            send(src_socket, client_request, MAX_LENGTH, 0);
            send(src_socket, "SOURCE", MAX_LENGTH, 0);
            send(src_socket, src_path, MAX_LENGTH, 0);
            int num_of_packets;
            recv(src_socket, &num_of_packets, sizeof(int), 0);
            char** file_content = (char**)malloc(MAX_PACKETS * sizeof(char*));
            for(int i = 0; i < MAX_LENGTH; i++){
                file_content[i] = (char*)malloc(MAX_LENGTH * sizeof(char));
            }
            for(int i = 0; i < num_of_packets; i++){
                recv(src_socket, file_content[i], MAX_LENGTH, 0);
            }
            printf("File content received successfully\n");
            fprintf(file, "Client request: %s - Received from source ss port: %d, ip: %s\n", client_request, src_port, src_ip);
            int dest_socket = create_client_socket(dest_ip, dest_port);
            send(dest_socket, client_request, MAX_LENGTH, 0);
            send(dest_socket, "DESTINATION", MAX_LENGTH, 0);
            char *filename = strrchr(src_path, '/');
            // Ensure there's a '/' at the end of dest_path
            size_t dest_len = strlen(dest_path);
            if (dest_path[dest_len - 1] != '/') {
                strcat(dest_path, "/");
            }
            // Append the filename to dest_path
            strcat(dest_path, filename + 1);
            // printf("%s\n", dest_path);
            send(dest_socket, dest_path, MAX_LENGTH, 0);
            send(dest_socket, &num_of_packets, sizeof(int), 0);
            for(int i = 0 ; i < num_of_packets; i++){
                send(dest_socket, file_content[i], MAX_LENGTH, 0);
            }
            printf("Sent successfully\n");
            char ack_message[MAX_LENGTH];
            recv(dest_socket, ack_message, MAX_LENGTH, 0);
            printf("%s\n", ack_message);
            for(int i = 0; i < MAX_LENGTH; i++){
                free(file_content[i]);
            }
            free(file_content);
            send(client_socket, "Copied successfully", MAX_LENGTH, 0);
            fprintf(file, "Client request: %s - Received ack from destination ss port: %d, ip: %s\n", client_request, dest_port, dest_ip);
            storage_servers[ss_no].accessible_paths[storage_servers[ss_no].path_count] = malloc(MAX_LENGTH * sizeof(char));
            strcpy(storage_servers[ss_no].accessible_paths[storage_servers[ss_no].path_count++], dest_path);
            insert_path(storage_servers[ss_no].path_trie, dest_path, &storage_servers[ss_no]);
        }
    } else {
        perror("Failed to read client request");
        error_code = 9;
        send(client_socket, &error_code, sizeof(int), 0);
    }
    close(client_socket);
    fclose(file);
}