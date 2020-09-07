#include "headers.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9890

int socket_desc, file_size, file_desc;
struct sockaddr_in server;
char reply_msg[BUFSIZ], filename[BUFSIZ], *data;

void sigintHandler(int sig_num)
{
	fflush(stdout);
	printf("!!!!Closing socket!!!!\n");
	close(socket_desc);
	exit(0);
}

int main(int argc, char **argv)
{
	signal(SIGINT, sigintHandler);

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	int a = 1;
	setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &a, sizeof(int));
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
		if (strcmp(filename, "exit") == 0 || strcmp(filename, "quit") == 0 || strcmp(filename, "q") == 0)
		{
			printf("\n!!!!Exiting!!!!!\n");
			char msg[] = "exit";
			write(socket_desc, msg, strlen(msg));
			close(socket_desc);
			break;
		}
		write(socket_desc, filename, strlen(filename));
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
			printf("\n!!!!File was Not Found on the server!!!!\n");
		}
	}

	return 0;
}