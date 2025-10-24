#include "headers.h"

int read_file(const char* path, int client_socket)
{
    printf("%s",path);
    FILE* file_pointer= fopen(path, "r");

    // Check if the file opened successfully.
    if(file_pointer == NULL){
        printf("Could not open file.\n");
        return -1;
    }
    else{
        printf("File opened successfully\n");
    }            
    char** file_data = (char**)malloc(MAX_PACKETS * sizeof(char*));
    for(int i = 0; i < MAX_LENGTH; i++){
        file_data[i] = (char*)malloc(MAX_LENGTH * sizeof(char));
    }
    int num_of_packets = 0;
    // fgets(file_content, MAX_LENGTH, file_pointer);
    while(fgets(file_data[num_of_packets], MAX_LENGTH, file_pointer) != NULL) {
        num_of_packets++;
    }
    send(client_socket, &num_of_packets, sizeof(int), 0);
    for(int i =0 ; i < num_of_packets; i++){
        send(client_socket, file_data[i], MAX_LENGTH, 0);
    }
    fclose(file_pointer);
    for(int i = 0; i < MAX_LENGTH; i++){
        free(file_data[i]);
    }
    free(file_data);
    printf("File Closed successfully\n");
    return 0;
}