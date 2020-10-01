#include "headers.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9890
#define CLIENT_FILES "client-files/"

int socket_desc, file_size, file_desc;
struct sockaddr_in server;
char reply_msg[BUFSIZ], filename[BUFSIZ], *data;

void sigintHandler(int sig_num) // function to handle smooth closing of the socket
{
	fflush(stdout);
	printf("!!!!Closing socket!!!!\n");
	char msg[] = "exit";
	// write(socket_desc, msg, strlen(msg)); // sending info to server that client is exiting
	close(socket_desc);
	exit(0);
}

int main(int argc, char **argv)
{
	// setting a system call on the keyboard interrupts so that the socket closes safely in any situation
	signal(SIGINT, sigintHandler);

	// creating a socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);

	// setting the propery of reusability on the socket port to get rid of the bind error
	int a = 1;
	setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &a, sizeof(int));

	if (socket_desc == -1)
	{
		perror("Could not create socket");
		return 1;
	}

	// setting socket properties
	server.sin_family = AF_INET;				   // to use ipv4 mechanism
	server.sin_addr.s_addr = inet_addr(SERVER_IP); // to accepts client from any ip address
	server.sin_port = htons(SERVER_PORT);		   // assigning port

	printf("\nSocket Created\n");

	if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Connection failed");
		return 1;
	}

	printf("\nSocket Connected\n");

	while (1)
	{
		// taking file name from the user
		printf("\nPlease enter file name to search: ");
		scanf("%s", filename);

		// if the user wishes to exit
		if (strcmp(filename, "exit") == 0 || strcmp(filename, "quit") == 0 || strcmp(filename, "q") == 0)
		{
			printf("\n!!!!Exiting!!!!!\n");
			sigintHandler(0);
			break;
		}
		// sending file name to the server
		write(socket_desc, filename, strlen(filename));

		// receiving the response from server whether the file exists or not
		recv(socket_desc, reply_msg, 2, 0);

		if (strcmp(reply_msg, "OK") == 0)
		{
			// receiving the file size of the requested file
			recv(socket_desc, &file_size, sizeof(int), 0);

			// making the file to get stored inside the clients direcotry only
			data = malloc(file_size);
			char location[BUFSIZ];
			strcpy(location, CLIENT_FILES);
			strcat(location, filename);
			file_desc = open(location, O_CREAT | O_EXCL | O_WRONLY, 0666);

			// receiving data inside the file
			recv(socket_desc, data, file_size, 0);

			// writing the data to the above created file
			write(file_desc, data, file_size);
			close(file_desc);
			printf("\nFile Received of size %d\n", file_size);
		}
		else
		{
			printf("\n!!!!File was Not Found on the server!!!!\n");
		}
	}

	return 0;
}