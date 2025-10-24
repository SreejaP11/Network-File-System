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
#include <time.h>

#define MAX_LENGTH 1024
#define MAX_LENGTH_OF_PATH 50
#define MAX_PACKETS 4096

typedef struct FileInfo {
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
extern int error_code;
// int read_file(const char *path, char** file_content, int* num_of_packets);
int read_file(const char* path, int client_socket);
int write_file(const char* path, char* data_from_client);
FileInfo additional_information_of_file(const char* file_name);
char** listing_all_files_and_folders(const char *folder_name, int *count);
FolderInfo additional_information_of_folder(char folder_name[MAX_LENGTH]);
int streaming_audio_file(char* path, int client_socket);
int copy_file(const char* source_path, const char* destination_path);
int is_path_accessible(const char *path);
int create_directory(const char *path);
int create_file(const char *path);
int delete_directory(const char *path);
int delete_path(const char *path);
void handle_command(int sock, const char *command);
