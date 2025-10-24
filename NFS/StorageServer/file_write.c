#include "headers.h"

int write_file(const char* path, char* data_from_client)
{
    printf("%s\n",path);
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    fprintf(file, "%s\n", data_from_client);
    fclose(file);
    printf("File write successfully completed\n");
    return 0;
}
