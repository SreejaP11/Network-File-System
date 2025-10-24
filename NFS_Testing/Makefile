all: 
	gcc client/main.c client/socket_utils.c client/client_oper.c -o client/run_client
	gcc NamingServer/naming_server.c NamingServer/main.c NamingServer/socket_utils.c NamingServer/handle_client.c NamingServer/store_paths.c -o NamingServer/run_naming_server
	gcc StorageServer/storage_server.c StorageServer/socket_utils.c StorageServer/main.c StorageServer/file_oper.c StorageServer/file_read.c StorageServer/file_write.c StorageServer/file_info.c StorageServer/list_of_folder.c StorageServer/folder_info.c StorageServer/file_stream.c -o StorageServer/run_storage_server

clean:
	rm StorageServer/run_storage_server*
	rm NamingServer/run_naming_server
	rm client/run_client
	