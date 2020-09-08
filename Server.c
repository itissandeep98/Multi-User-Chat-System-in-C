#include "headers.h"

#define SERVER_PORT 9890
#define SERVER_FILES "server-files/"

int socket_desc, socket_client, *new_sock, c = sizeof(struct sockaddr_in);

void sigintHandler(int sig_num) // function to handle smooth closing of the socket
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

	// Recieve File name from the connected client
	recv(socket, file_name, BUFSIZ, 0);

	if (strcmp(file_name, "exit") == 0 || strcmp(file_name, "") == 0) // compare the data recieved from client if it wants to exit
	{
		printf("\nClient Disconnected\n");
		return 0;
	}

	// since server files are located inside its directory so concatinating that location with the file name
	char location[BUFSIZ];
	strcpy(location, SERVER_FILES);
	strcat(location, file_name);

	strcpy(file_name, "");

	FILE *fp = fopen(location, "r");
	if (fp != NULL) // checking if the file exists or not
	{
		printf("\nFile Found: %s\n", location);

		// sending a response to client that file exists on server
		strcpy(server_response, "OK");
		write(socket, server_response, strlen(server_response));

		// Obtaining the file size and sending to client to excepect this size of file
		stat(location, &obj);
		file_desc = open(location, O_RDONLY);
		file_size = obj.st_size;
		send(socket, &file_size, sizeof(int), 0);

		// Sending the file
		sendfile(socket, file_desc, NULL, file_size);

		printf("\nFile Sent\n ");

		free(fp);
		return 1;
	}
	else
	{
		// Showing error if the file was not found
		perror("\nFile Not found\n");
		strcpy(server_response, "NO");
		// sending a NO response to the client
		write(socket, server_response, strlen(server_response));
		return 1;
	}
}

int main()
{
	// setting a system call on the keyboard interrupts so that the socket closes safely in any situation
	signal(SIGINT, sigintHandler);

	struct sockaddr_in server, client;
	// creating a socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	
	// setting the propery of reusability on the socket port to get rid of the bind error
	int a = 1;
	setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &a, sizeof(int));

	if (socket_desc == -1)
	{
		perror("\nCould not create socket");
		return 1;
	}

	// setting socket properties
	server.sin_family = AF_INET;		  // to use ipv4 mechanism
	server.sin_addr.s_addr = INADDR_ANY;  // to accepts client from any ip address
	server.sin_port = htons(SERVER_PORT); // assigning port

	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) // binding the socket
	{
		perror("\nBind failed");
		return 1;
	}

	// listening on port
	listen(socket_desc, 3);

	while (socket_client = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)) // loop runs until there is a new client
	{
		printf("\nClient Connected\n");
		new_sock = malloc(1);
		*new_sock = socket_client;

		while (HandleConn(new_sock)); // runs until the client exits
		close(socket_client);
	}

	if (socket_client < 0)
	{
		perror("\nAccept failed");
		return 1;
	}

	return 0;
}
