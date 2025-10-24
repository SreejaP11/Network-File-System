#include "storage_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // For directory operations
#include <sys/stat.h> // For file type checks
#include <unistd.h> // For access

AccessiblePaths accessible_paths;

int is_path_duplicate(const char *path) {
    for (int i = 0; i < accessible_paths.path_count; i++) {
        if (strcmp(accessible_paths.paths[i], path) == 0) {
            return 1; // Path is a duplicate
        }
    }
    return 0; // Path is not a duplicate
}

void add_path(const char *path) {
    if (accessible_paths.path_count >= MAX_PATHS) {
        fprintf(stderr, "Error: Too many paths. Maximum allowed is %d.\n", MAX_PATHS);
        return;
    }

    if (is_path_duplicate(path)) {
        // printf("Duplicate path detected: %s. Skipping addition.\n", path);
        return;
    }

    accessible_paths.paths[accessible_paths.path_count] = (char*) malloc(PATH_MAX_LEN * sizeof(char*));

    strncpy(accessible_paths.paths[accessible_paths.path_count], path, PATH_MAX_LEN - 1);
    accessible_paths.paths[accessible_paths.path_count][PATH_MAX_LEN - 1] = '\0'; // Ensure null-termination
    accessible_paths.path_count++;
}

void collect_child_paths(const char *base_path) {
    DIR *dir = opendir(base_path);
    if (dir == NULL) {
        perror("Failed to open directory");
        return;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Skip the current and parent directories
        }

        char child_path[PATH_MAX_LEN];
        int ret = snprintf(child_path, sizeof(child_path), "%s/%s", base_path, entry->d_name);
        if (ret >= sizeof(child_path)) {
            fprintf(stderr, "Warning: Path truncated: %s/%s\n", base_path, entry->d_name);
            continue;
        }

        struct stat path_stat;
        if (stat(child_path, &path_stat) == -1) {
            perror("Failed to stat file");
            continue;
        }

        // Add the child path
        add_path(child_path);

        // If the entry is a directory, recursively collect its children
        if (S_ISDIR(path_stat.st_mode)) {
            collect_child_paths(child_path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 7) { // Minimum arguments: program name, NM_IP, NM_Port, Client_Port, Server_Port, SS_IP, and at least one accessible path
        fprintf(stderr, "Usage: %s <NM_IP> <NM_Port> <Client_Port> <Server_Port> <SS_IP> <Path1> [Path2] ...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *nm_ip = argv[1];
    int nm_port = atoi(argv[2]);
    int client_port = atoi(argv[3]);
    int server_port = atoi(argv[4]);
    char* ss_ip = argv[5];

    // Initialize accessible paths
    accessible_paths.path_count = 0;

    // Add provided paths and their children
    for (int i = 6; i < argc; i++) {
        add_path(argv[i]); // Add the base path
        collect_child_paths(argv[i]); // Add child paths
    }
    // Call initialization function with the accessible paths
    initialize_storage_server(nm_ip, nm_port, ss_ip, client_port, server_port, &accessible_paths);

    return 0;
}
