#include "headers.h"

#define SERVER_PORT 2001
#define IP_ADDRESS "127.0.0.1" // localhost

void *ConnectionHandler(void *socket_desc);
bool SendFileOverSocket(int socket_desc, char *file_name);

int main(int argc, char **argv)
{
	int socket_desc, socket_client, *new_sock, c = sizeof(struct sockaddr_in);

	struct sockaddr_in server, client;

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_desc == -1)
	{
		perror("Could not create socket");
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	server.sin_port = htons(SERVER_PORT);

	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Bind failed");
		return 1;
	}

	listen(socket_desc, 3);

	if (socket_client = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c))
	{
		// pthread_t sniffer_thread;
		printf("\nconnected\n");
		new_sock = malloc(1);
		*new_sock = socket_client;
		// pthread_create(&sniffer_thread, NULL, ConnectionHandler, (void *)new_sock);
		// pthread_join(sniffer_thread, NULL);
		while (1)
		{
			ConnectionHandler(new_sock);
		}
	}

	if (socket_client < 0)
	{
		perror("Accept failed");
		return 1;
	}

	return 0;
}

void *ConnectionHandler(void *socket_desc)
{
	int socket = *(int *)socket_desc;
	char server_response[BUFSIZ], client_request[BUFSIZ], file_name[BUFSIZ];

	recv(socket, client_request, BUFSIZ, 0);
	strcpy(file_name, client_request);

	char location[BUFSIZ];
	strcpy(location, "server-files/");
	strcat(location, file_name);

	FILE *fp = fopen(location, "r");
	if (fp != NULL)
	{
		printf("\nfile found\n ");
		strcpy(server_response, "OK");
		write(socket, server_response, strlen(server_response));
		SendFileOverSocket(socket, location);
	}
	else
	{
		perror("\nfile Not found ");
		strcpy(server_response, "NO");
		write(socket, server_response, strlen(server_response));
	}

	free(fp);
	return 0;
}

bool SendFileOverSocket(int socket_desc, char *file_name)
{

	struct stat obj;
	int file_desc, file_size;

	stat(file_name, &obj);
	file_desc = open(file_name, O_RDONLY);
	file_size = obj.st_size;
	send(socket_desc, &file_size, sizeof(int), 0);
	sendfile(socket_desc, file_desc, NULL, file_size);

	return true;
}