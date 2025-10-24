#include <stdio.h>
#include <stdlib.h>
#include "naming_server.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <NM_IP>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // converting the port number (string to integer)
    int port = atoi(argv[1]);
    char* ip = argv[2];
    if (port <= 0) {
        fprintf(stderr, "Invalid port number. Please provide a valid port.\n");
        exit(EXIT_FAILURE);
    }
    // start
    printf("Starting Naming Server on port %d...\n", port);
    initialize_naming_server(ip, port);
    // connect_with_clients(ip, port);

    return 0;
}
