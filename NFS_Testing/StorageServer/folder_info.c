#include "headers.h"

// Function to calculate the total size of a folder
long long calculateTotalSize(const char *folderPath) {
    struct stat st;
    if (lstat(folderPath, &st) != 0) {
        perror("Error getting folder information");
        error_code = 3;
        return 0;
    }

    if (!S_ISDIR(st.st_mode)) {
        // It's a file
        return st.st_size;
    }

    // It's a directory, so calculate the total size
    long long totalSize = 0;
    DIR *d = opendir(folderPath);
    struct dirent *entry;
    if (!d) {
        perror("Error opening folder");
        error_code = 11;
        return 0;
    }

    while ((entry = readdir(d))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[MAX_LENGTH];
        snprintf(path, sizeof(path), "%s/%s", folderPath, entry->d_name);
        totalSize += calculateTotalSize(path);
    }

    closedir(d);
    return totalSize;
}

void countFolderContents(const char *folderPath, int *numFiles, int *numSubfolders) {
    DIR *d = opendir(folderPath);

    if (!d) {
        perror("Error opening folder");
        error_code = 11;
        return;
    }

    struct dirent *entry;
    *numFiles = 0;
    *numSubfolders = 0;

    while ((entry = readdir(d)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                (*numSubfolders)++;
            }
        } else {
            (*numFiles)++;
        }
    }

    closedir(d);
}

FolderInfo additional_information_of_folder(char folder_name[MAX_LENGTH]) { 
    FolderInfo folder_details;
    char folderPath[MAX_LENGTH];
    strcpy(folderPath, folder_name);
    
    // Folder name
    strcpy(folder_details.folder_name, folder_name);
    printf("Folder Name: %s\n", folder_details.folder_name);

    // Folder information
    struct stat st;
    if (lstat(folderPath, &st) == 0) {
        printf("Folder information fetched successfully\n");
        folder_details.valid = 1;
        folder_details.size = (int)st.st_size;

        // Permissions (assign to Permissions field)
        snprintf(folder_details.Permissions, sizeof(folder_details.Permissions), "%c%c%c%c%c%c%c%c%c%c",
            (S_ISDIR(st.st_mode)) ? 'd' : '-',
            (st.st_mode & S_IRUSR) ? 'r' : '-',
            (st.st_mode & S_IWUSR) ? 'w' : '-',
            (st.st_mode & S_IXUSR) ? 'x' : '-',
            (st.st_mode & S_IRGRP) ? 'r' : '-',
            (st.st_mode & S_IWGRP) ? 'w' : '-',
            (st.st_mode & S_IXGRP) ? 'x' : '-',
            (st.st_mode & S_IROTH) ? 'r' : '-',
            (st.st_mode & S_IWOTH) ? 'w' : '-',
            (st.st_mode & S_IXOTH) ? 'x' : '-');

        // Last Modified Time (assign to last_modified_time field)
        snprintf(folder_details.last_modified_time, sizeof(folder_details.last_modified_time), "%s", ctime(&st.st_mtime));

        // Created Time (assign to created_time field)
        snprintf(folder_details.created_time, sizeof(folder_details.created_time), "%s", ctime(&st.st_ctime));

        // Parent folder (assign to parent_folder field)
        char parentFolder[MAX_LENGTH_OF_PATH];
        if (realpath(folderPath, parentFolder) != NULL) {
            char *lastSlash = strrchr(parentFolder, '/');
            if (lastSlash != NULL) {
                *lastSlash = '\0';
                snprintf(folder_details.parent_folder, sizeof(folder_details.parent_folder), "%s", parentFolder);
            }
        } else {
            perror("Error getting parent folder information");
            error_code = 12;
            snprintf(folder_details.parent_folder, sizeof(folder_details.parent_folder), "N/A");
        }
    } else {
        perror("Error getting folder information");
        error_code = 3;
        folder_details.valid = 0;
    }

    // Calculate and print total size
    long long totalSize = calculateTotalSize(folderPath);
    if (totalSize > 0) {
        // Convert bytes to kilobytes (KB)
        folder_details.total_size_of_folder_in_KB = totalSize / 1024.0;
    } else {
        printf("Error calculating total size.\n");
        folder_details.total_size_of_folder_in_KB = 0;
    }
    
    // Count files and subfolders
    int numFiles, numSubfolders;
    countFolderContents(folderPath, &numFiles, &numSubfolders);

    folder_details.total_files = numFiles;
    folder_details.total_folders = numSubfolders;

    return folder_details;
}

