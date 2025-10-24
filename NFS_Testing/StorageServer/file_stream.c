#include "headers.h"

int streaming_audio_file(char* path, int client_socket)
{
    FILE *audio_file = fopen(path, "rb");
    if (audio_file == NULL) {
        perror("Error opening audio file");
        send(client_socket, "Error opening audio file", MAX_LENGTH, 0);
        free(path);
        close(client_socket);
        return -1;
    }

    char buffer[MAX_LENGTH];
    size_t bytes_read;

    printf("Streaming audio file: %s\n", path);
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), audio_file)) > 0) {
        send(client_socket, buffer, bytes_read, 0); // Send audio data in chunks
    }

    fclose(audio_file);
    free(path);
    printf("Audio file streaming completed.\n");
    return 0;
}