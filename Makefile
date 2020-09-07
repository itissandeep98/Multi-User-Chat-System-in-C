all:Server.c
	gcc -pthread Server.c -o server
	./server || true

client:
	gcc -pthread Client.c -o client
	./client || true


clean:
	rm -f client server *.txt
