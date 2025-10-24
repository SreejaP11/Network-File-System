#include "headers.h"

FileInfo additional_information_of_file(const char* file_name)
{
    printf("Additional information of the file.\n");

    FileInfo file_info = {0}; // Initialize the struct with default values
    struct stat file_stat;

    if (stat(file_name, &file_stat) == -1) {
        printf("Error getting file information\n");
        error_code = 4;
        return file_info; // Return an empty struct if there is an error
    }
    else {
        printf("File details fetched successfully.\n");
        file_info.valid = 1;
        file_info.file_size = file_stat.st_size;
        file_info.file_permissions = file_stat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
        file_info.last_access_time = file_stat.st_atime;
        file_info.last_modification_time = file_stat.st_mtime;
        file_info.last_status_change_time = file_stat.st_ctime;
        file_info.file_owner = file_stat.st_uid;
        file_info.file_group = file_stat.st_gid;
        file_info.file_inode_number = file_stat.st_ino;
        file_info.file_device_id = file_stat.st_dev;

        // File extension
        const char *file_extension = strrchr(file_name, '.');
        if (file_extension) {
            strncpy(file_info.file_extension, file_extension, MAX_LENGTH - 1);
            file_info.file_extension[MAX_LENGTH - 1] = '\0'; // Ensure null termination
        } else {
            strcpy(file_info.file_extension, "Not found");
        }

        // Determine file type based on the extension
        if (file_extension) {
            if (strcmp(file_extension, ".txt") == 0) {
                strcpy(file_info.file_type, "Text File");
            } else if (strcmp(file_extension, ".jpg") == 0 || strcmp(file_extension, ".jpeg") == 0) {
                strcpy(file_info.file_type, "JPEG Image");
            } else {
                strcpy(file_info.file_type, "Unknown");
            }
        } else {
            strcpy(file_info.file_type, "Unknown");
        }

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
        printf("File information sent successfully\n");
    }
    return file_info;
}
