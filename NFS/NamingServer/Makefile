CC = gcc
CFLAGS = -Wall -pthread

all: naming_server

naming_server: main.o naming_server.o socket_utils.o file_oper.o
	$(CC) $(CFLAGS) -o naming_server main.o naming_server.o socket_utils.o file_oper.o

main.o: main.c naming_server.h
	$(CC) $(CFLAGS) -c main.c

naming_server.o: naming_server.c naming_server.h socket_utils.h file_oper.h
	$(CC) $(CFLAGS) -c naming_server.c

socket_utils.o: socket_utils.c socket_utils.h
	$(CC) $(CFLAGS) -c socket_utils.c

file_oper.o: file_oper.c file_oper.h
	$(CC) $(CFLAGS) -c file_oper.c

clean:
	rm -f *.o naming_server
