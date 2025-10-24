#include "client.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <NM_port> <NM_IP>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // converting the port number (string to integer)
    int port = atoi(argv[1]);
    char* ip = argv[2];
    if (port <= 0) {
        fprintf(stderr, "Invalid port number. Please provide a valid port.\n");
        exit(EXIT_FAILURE);
    }

    int sock = create_client_socket(ip, port);
    if (sock < 0) {
        perror("Failed to connect to Naming Server");
        exit(EXIT_FAILURE);
    }    
    else{
        printf("Connected to the server.\n\n");
    }
    send(sock, "CLIENT", MAX_LENGTH, 0);
    char client_operation[MAX_LENGTH];
    printf("Enter operation (or type 'EXIT' to quit): ");
    if (fgets(client_operation, MAX_LENGTH, stdin) == NULL) {
        perror("Error reading input");
        return 0;
    }

    // Remove trailing newline character
    size_t len = strlen(client_operation);
    if (len > 0 && client_operation[len - 1] == '\n') {
        client_operation[len - 1] = '\0';
    }

    // Check if the user wants to exit
    if (strcasecmp(client_operation, "EXIT") == 0) {
        printf("Exiting...\n");
        return 0;
    }

    // Send the operation to the server
    if (send(sock, client_operation, MAX_LENGTH, 0) < 0) {
        perror("Failed to send operation to the server");
        return 0;
    }
    handle_operations(sock, client_operation);
    close(sock);
    return 0;
}
