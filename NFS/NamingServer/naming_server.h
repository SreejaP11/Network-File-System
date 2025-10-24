#ifndef NAMING_SERVER_H
#define NAMING_SERVER_H

#include <netinet/in.h>

#define MAX_CLIENTS 5
#define MAX_SERVERS 10
#define PING_INTERVAL 5  
#define BUFFER_SIZE 1024
#define MAX_PATHS 100
#define PATH_MAX_LEN 256
#define MAX_LENGTH 1024
#define MAX_SS_LENGTH 2048
#define MAX_PACKETS 4096
#define FILENAME "book_keeping.txt"

typedef struct SSInfo{
    char ss_ip[32];
    int client_port;  
    int server_port;  
    char accessible_paths[MAX_LENGTH];
}SSInfo;

typedef struct StorageServer {
    SSInfo ssinfo;
    int is_active;
    int path_count;
    char** accessible_paths;
    struct TrieNode* path_trie;  // Root of the Trie for accessible paths
    int server_socket;
} StorageServer;

typedef struct TrieNode {
    char name[PATH_MAX_LEN];
    int is_directory;
    struct TrieNode** children;
    int child_count;
    int max_children;
    StorageServer* storage_server;  // Pointer to the associated storage server
} TrieNode;

typedef struct SSInfo_Client{
    int client_port;
    char ip[32];
}SSInfo_Client;

extern StorageServer storage_servers[MAX_SERVERS];
extern int server_count;
extern int error_code;

void initialize_naming_server(char* ip, int port);
void handle_client_connection(int client_socket);
void handle_storage_server_registration(int server_socket);
void *monitor_storage_servers(void *arg);
void handle_client_request(int client_socket);
TrieNode* create_trie_node(const char* name);
char** split_path(const char* path, int* count);
void insert_path(TrieNode* root, const char* path, StorageServer* server);
StorageServer* search_path(TrieNode* root, const char* path);
int is_path_duplicate(TrieNode* root, const char* path);
int remove_path_recursive(TrieNode* current, char** components, int depth, int component_count);
int remove_path(TrieNode* root, const char* path);
void free_trie(TrieNode* root);
void print_paths_recursive(TrieNode* node, char* path, int depth);
void print_all_paths(TrieNode* root);

#endif
