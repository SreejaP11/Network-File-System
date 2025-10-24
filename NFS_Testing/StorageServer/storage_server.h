#ifndef STORAGE_SERVER_H
#define STORAGE_SERVER_H
#define MAX_PATHS 1000
#define BUFFER_SIZE 1024
#define PATH_MAX_LEN 256
#define MAX_LENGTH 1024
#define MAX_SS_LENGTH 8192
#define MAX_FILE_LENGTH 4096
#define MAX_PACKETS 4096

#include <netinet/in.h>

typedef struct {
    char *paths[MAX_PATHS]; 
    int path_count;          
} AccessiblePaths;

typedef struct StorageServerInfo{
    char ip[32];      // SS IP
    int client_port;      // For client connection
    int nm_port;          // For NM connection
    char accessible_paths[MAX_LENGTH];   // Accessible paths in SS
}StorageServerInfo;

typedef struct ListFolder{
    int count;
    char** paths_list;
}ListFolder;

extern AccessiblePaths accessible_paths;

void initialize_storage_server(const char *nm_ip, int nm_port, char* ss_ip, int client_port, int server_port, AccessiblePaths *accessible_paths);
void register_with_naming_server(const char *nm_ip, int nm_port, char* ss_ip, int client_port, int server_port, AccessiblePaths *accessible_paths);
void handle_naming_server_requests(int *server_socket);
void handle_client_requests(int *client_socket);
void handle_single_client(int *client_socket_ptr);

#endif
