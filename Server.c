#include "headers.h"

#define SERVER_PORT 9890
#define IP_ADDRESS "127.0.0.1" // localhost

int socket_desc, socket_client, *new_sock, c = sizeof(struct sockaddr_in);

void sigintHandler(int sig_num)
{
	fflush(stdout);
	printf("!!!!Closing socket!!!!\n");
	close(socket_desc);
	exit(0);
}

int HandleConn(void *socket_desc)
{
	int socket = *(int *)socket_desc;
	char server_response[BUFSIZ], file_name[BUFSIZ];
	struct stat obj;
	int file_desc, file_size;

	recv(socket, file_name, BUFSIZ, 0);
	if (strcmp(file_name, "exit") == 0 || strcmp(file_name, "") == 0)
	{
		printf("\nClient Disconnected\n\n!!Exiting!!\n");
		sigintHandler(0);
		return 0;
	}

	char location[BUFSIZ];
	strcpy(location, "server-files/");
	strcat(location, file_name);

	strcpy(file_name,"");

	FILE *fp = fopen(location, "r");
	if (fp != NULL)
	{
		printf("\nFile Found: %s\n", location);

		strcpy(server_response, "OK");
		write(socket, server_response, strlen(server_response));

		stat(location, &obj);
		file_desc = open(location, O_RDONLY);
		file_size = obj.st_size;
		send(socket, &file_size, sizeof(int), 0);
		sendfile(socket, file_desc, NULL, file_size);

		printf("\nFile Sent\n ");

		free(fp);
		return 1;
	}
	else
	{
		perror("\nfile Not found ");
		strcpy(server_response, "NO");
		write(socket, server_response, strlen(server_response));
		return 1;
	}
}

int main()
{
	signal(SIGINT, sigintHandler);

	struct sockaddr_in server, client;

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	int a = 1;
	setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &a, sizeof(int));

	if (socket_desc == -1)
	{
		perror("\nCould not create socket");
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(SERVER_PORT);

	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("\nBind failed");
		return 1;
	}

	listen(socket_desc, 3);

	if (socket_client = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c))
	{
		printf("\nClient Connected\n");
		new_sock = malloc(1);
		*new_sock = socket_client;

		while (HandleConn(new_sock))
			;
	}

	if (socket_client < 0)
	{
		perror("\nAccept failed");
		return 1;
	}

	return 0;
}
