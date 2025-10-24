#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h> 
#include<errno.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>

#define MAX_LENGTH 1024
#define MAX_FILE_LENGTH 4096
#define MAX_PACKETS 4096
#define MAX_PATHS 1000
#define BUF_LEN 8000

typedef struct SSInfo{
    int port;
    char ip[32];
}SSInfo;

typedef struct FolderInfo{
    int valid;
    char folder_name[MAX_LENGTH];
    int size; 
    char Permissions[MAX_LENGTH];
    char last_modified_time[MAX_LENGTH];
    char created_time[MAX_LENGTH];
    char parent_folder[MAX_LENGTH];
    float total_size_of_folder_in_KB;
    int total_files;
    int total_folders;
}FolderInfo;

typedef struct FileInfo{
    int valid;
    off_t file_size;
    mode_t file_permissions;
    time_t last_access_time;
    time_t last_modification_time;
    time_t last_status_change_time;
    char file_extension[MAX_LENGTH];
    char file_type[MAX_LENGTH];
    uid_t file_owner;
    gid_t file_group;
    ino_t file_inode_number;
    dev_t file_device_id;
} FileInfo;

typedef struct ListFolder{
    int count;
    char** paths_list;
}ListFolder;

int create_client_socket(const char *ip, int port);
void handle_operations(int sock, char* client_operation);
