#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

int create_server_socket(char* ip, int port, int backlog);
int create_client_socket(const char *ip, int port);

#endif
