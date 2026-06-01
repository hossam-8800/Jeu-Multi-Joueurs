all: server client

server: server.c common.h
	gcc server.c -o server

client: client.c common.h
	gcc client.c -o client -lSDL2

clean:
	rm -f server client server.exe client.exe