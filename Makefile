all:Server.c
	gcc -pthread Server.c -o server && ./server

client:
	gcc -pthread Client.c -o client && ./client


clean:
	rm -f client server
