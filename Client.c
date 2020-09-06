#include "headers.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2001

int main(int argc, char **argv)
{
	int socket_desc;
	struct sockaddr_in server;
	char request_msg[BUFSIZ], reply_msg[BUFSIZ];

	int file_size, file_desc;
	char *data;
	char filename[64];

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		perror("Could not create socket");
		return 1;
	}

	server.sin_addr.s_addr = inet_addr(SERVER_IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);

	if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Connection failed");
		return 1;
	}
	while (1)
	{
		printf("\nPlease enter file name to search:\n");
		scanf("%s", filename);
		strcpy(request_msg, filename);
		write(socket_desc, request_msg, strlen(request_msg));
		recv(socket_desc, reply_msg, 2, 0);

		if (strcmp(reply_msg, "OK") == 0)
		{

			recv(socket_desc, &file_size, sizeof(int), 0);
			data = malloc(file_size);
			file_desc = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
			recv(socket_desc, data, file_size, 0);
			write(file_desc, data, file_size);
			close(file_desc);
		}
		else
		{

			fprintf(stderr, "Bad request\n");
		}
	}

	return 0;
}