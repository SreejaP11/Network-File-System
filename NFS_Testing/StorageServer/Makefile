CC = gcc
CFLAGS = -Wall -pthread

all: storage_server

storage_server: main.o storage_server.o socket_utils.o file_oper.o
	$(CC) $(CFLAGS) -o storage_server main.o storage_server.o socket_utils.o file_oper.o

main.o: main.c storage_server.h file_oper.h
	$(CC) $(CFLAGS) -c main.c

storage_server.o: storage_server.c storage_server.h socket_utils.h
	$(CC) $(CFLAGS) -c storage_server.c

socket_utils.o: ../NamingServer/socket_utils.c ../NamingServer/socket_utils.h
	$(CC) $(CFLAGS) -c ../NamingServer/socket_utils.c -o socket_utils.o

file_oper.o: file_oper.c file_oper.h
	$(CC) $(CFLAGS) -c file_oper.c

clean:
	rm -f *.o storage_server
