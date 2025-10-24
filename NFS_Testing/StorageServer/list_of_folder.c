#include "headers.h"

char** listing_all_files_and_folders(const char *folder_name, int *count) {
    struct dirent *entry;
    DIR *dir = opendir(folder_name);
    char **file_and_folder_names = NULL; // Pointer to store file and folder names
    *count = 0;

    if (dir == NULL) {
        perror("Error opening directory");
        error_code = 2;
        return NULL;
    }

    printf("Files and folders in %s:\n", folder_name);

    // Count the number of files and folders
    while ((entry = readdir(dir)) != NULL) {
        (*count)++;
    }

    // Allocate memory for storing the names
    file_and_folder_names = malloc((*count) * sizeof(char *));
    if (file_and_folder_names == NULL) {
        perror("Memory allocation failed");
        error_code = 5;
        closedir(dir);
        return NULL;
    }

    // Reset directory stream and read entries again to store names
    rewinddir(dir);
    int index = 0;
    while ((entry = readdir(dir)) != NULL) {
        file_and_folder_names[index] = malloc(strlen(entry->d_name) + 1); // Allocate memory for name
        if (file_and_folder_names[index] == NULL) {
            perror("Memory allocation failed for name");
            closedir(dir);

            // Free previously allocated memory
            for (int i = 0; i < index; i++) {
                free(file_and_folder_names[i]);
            }
            free(file_and_folder_names);
            return NULL;
        }
        strcpy(file_and_folder_names[index], entry->d_name); // Copy name
        printf("%s\n", file_and_folder_names[index]);
        index++;
    }

    closedir(dir);
    return file_and_folder_names;
}