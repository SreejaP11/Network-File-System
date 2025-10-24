#include "naming_server.h"
#include "socket_utils.h"
#include "colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BACKLOG 20
StorageServer storage_servers[MAX_SERVERS];
int server_count = 0;
int error_code = 0;
void initialize_naming_server(char* ip, int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    server_fd = create_server_socket(ip, port, BACKLOG);
    if (server_fd < 0) {
        perror("Server socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Naming Server is listening on port %d\n", port);

    pthread_t monitor_thread;
    if (pthread_create(&monitor_thread, NULL, monitor_storage_servers, NULL) != 0) {
        perror("Failed to create monitoring thread");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
        // Determine the type of connection (Storage Server or Client)
        char type[BUFFER_SIZE] = {0};
        int read_size = recv(new_socket, type, MAX_LENGTH, 0);

        if (read_size > 0) {
            type[read_size] = '\0';
            if (strcmp(type, "STORAGE_SERVER") == 0) {
                printf("Storage Server connected.\n");
                handle_storage_server_registration(new_socket);
            } else if (strcmp(type, "CLIENT") == 0) {
                printf("Client connected.\n");
                handle_client_request(new_socket); // Handle client requests
            } else {
                fprintf(stderr, "Unknown connection type: %s\n", type);
                close(new_socket);
            }
        } else {
            perror("Failed to read connection type");
            close(new_socket);
        }
    }

    close(server_fd);
}

void handle_storage_server_registration(int server_socket) {
    StorageServer new_server;
    memset(&new_server, 0, sizeof(new_server));
    new_server.is_active = 1;
    SSInfo ss_info = {0};
    int recv_size = recv(server_socket, &ss_info, MAX_SS_LENGTH, 0);
    if (recv_size <= 0) {
        perror("Failed to receive data");
        close(server_socket);
        return;
    }
    // Copy data to StorageServer structure
    strncpy(new_server.ssinfo.ss_ip, ss_info.ss_ip, sizeof(new_server.ssinfo.ss_ip) - 1);
    new_server.ssinfo.client_port = ss_info.client_port;
    new_server.ssinfo.server_port = ss_info.server_port;
    char buffer[BUF_LEN];
    strcpy(buffer, ss_info.accessible_paths);
    int read_size = strlen(buffer);
    new_server.path_trie = create_trie_node("/");
    if(read_size > 0) {
        buffer[read_size] = '\0';
        printf("Received message: %s\n", buffer);
        
        char *tokens[MAX_PATHS];
        char *token = strtok(buffer, "|");
        int token_count = 0;
        
        while (token != NULL && token_count < MAX_PATHS ) {
            tokens[token_count] = token;
            token_count++;
            token = strtok(NULL, "|");
        }
        new_server.accessible_paths = malloc(token_count * sizeof(char *));
        new_server.path_count = token_count;

        // Allocate and copy each token into accessible_paths
        for (int i = 0; i < token_count; i++) {
            new_server.accessible_paths[i] = malloc(strlen(tokens[i]) + 1);  // Allocate memory for each path
            strcpy(new_server.accessible_paths[i], tokens[i]);  // Copy the token (path) to accessible_paths
            insert_path(new_server.path_trie, new_server.accessible_paths[i], &new_server);
        }
    }

    if (server_count < MAX_SERVERS) {
        storage_servers[server_count++] = new_server;
        printf("Registered new storage server: IP %s, Client Port %d, Server Port %d\n", new_server.ssinfo.ss_ip, new_server.ssinfo.client_port, new_server.ssinfo.server_port);
    } else {
        fprintf(stderr, "Max storage servers reached, cannot register new server.\n");
    }
    close(server_socket);
}

void *monitor_storage_servers(void *arg) {
    while (1) {
        for (int i = 0; i < server_count; i++) {
            if (storage_servers[i].is_active) {
                storage_servers[i].server_socket = create_client_socket(storage_servers[i].ssinfo.ss_ip, storage_servers[i].ssinfo.server_port);
                if (storage_servers[i].server_socket < 0) {
                    fprintf(stderr, "Storage server %s:%d is inactive\n", storage_servers[i].ssinfo.ss_ip, storage_servers[i].ssinfo.server_port);
                    storage_servers[i].is_active = 0;
                } else {
                    // printf("Storage server %s:%d is active\n", storage_servers[i].ip, storage_servers[i].server_port);
                    close(storage_servers[i].server_socket);
                }
            }
        }
        sleep(PING_INTERVAL);
    }
    return NULL;
}
